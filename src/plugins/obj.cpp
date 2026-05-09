/*
 * Project: Raytracer
 * File name: obj.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: OBJ mesh primitive plugin. Parses .obj files and builds an internal
 *                   BVH (SAH binned) over the resulting triangles for O(log n) intersection.
 */

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "AABB.hpp"
#include "IMaterial.hpp"
#include "IPrimitive.hpp"
#include "Vec3.hpp"

namespace {

static constexpr int NUM_BINS = 12;
static constexpr int MAX_LEAF_SIZE = 4;

static double surfaceArea(const AABB &box) {
    Vec3 d = box.max - box.min;
    if (d.x < 0.0 || d.y < 0.0 || d.z < 0.0) return 0.0;
    return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

struct MeshFace {
    Vec3 v0, v1, v2;
    Vec3 n0, n1, n2;
    Vec3 edge1, edge2;
    Vec3 faceNormal;
    AABB bbox;
};

struct MeshNode {
    AABB box;
    int left = -1;
    int right = -1;
    int faceBegin = 0;
    int faceEnd = 0;
};

void parseFaceToken(const std::string &tok, int &vi, int &ni) {
    vi = 0;
    ni = 0;
    auto s1 = tok.find('/');
    if (s1 == std::string::npos) {
        vi = std::stoi(tok);
        return;
    }
    vi = std::stoi(tok.substr(0, s1));
    auto s2 = tok.find('/', s1 + 1);
    if (s2 != std::string::npos && s2 + 1 < tok.size())
        ni = std::stoi(tok.substr(s2 + 1));
}

int resolveIdx(int idx, int size) {
    return idx > 0 ? idx - 1 : size + idx;
}

} // namespace

class ObjMesh : public IPrimitive {
public:
    void setFilePath(const std::string &path) override { _filePath = path; }

    void configure(const std::unordered_map<std::string, double> &,
        std::shared_ptr<IMaterial> mat) override {
        _material = std::move(mat);
        if (!_filePath.empty())
            parseAndBuild();
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        if (_nodes.empty())
            return std::nullopt;
        double tMax = std::numeric_limits<double>::infinity();
        return traverseNode(0, ray, epsilon, tMax);
    }

    AABB boundingBox() const override { return _globalBox; }

private:
    std::string _filePath;
    std::shared_ptr<IMaterial> _material;
    std::vector<MeshFace> _faces;
    std::vector<int> _faceIdx;
    std::vector<MeshNode> _nodes;
    AABB _globalBox = AABB::empty();

    void parseAndBuild() {
        std::ifstream in(_filePath);
        if (!in)
            throw std::runtime_error("ObjMesh: cannot open '" + _filePath + "'");

        std::vector<Vec3> verts;
        std::vector<Vec3> norms;

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#')
                continue;
            std::istringstream ss(line);
            std::string tok;
            ss >> tok;

            if (tok == "v") {
                double x, y, z;
                ss >> x >> y >> z;
                verts.push_back({x, y, z});
            } else if (tok == "vn") {
                double x, y, z;
                ss >> x >> y >> z;
                norms.push_back({x, y, z});
            } else if (tok == "f") {
                std::vector<std::string> tokens;
                std::string t;
                while (ss >> t)
                    tokens.push_back(t);
                if (tokens.size() < 3)
                    continue;
                for (size_t i=1; i+1<tokens.size(); i++)
                    addFace(tokens[0], tokens[i], tokens[i + 1], verts, norms);
            }
        }

        if (!verts.empty()) {
            Vec3 min = verts[0];
            Vec3 max = verts[0];
            for (const auto &v : verts) {
                min.x = std::min(min.x, v.x);
                min.y = std::min(min.y, v.y);
                min.z = std::min(min.z, v.z);
                max.x = std::max(max.x, v.x);
                max.y = std::max(max.y, v.y);
                max.z = std::max(max.z, v.z);
            }
            _globalBox = AABB(min, max);
        } else
            _globalBox = AABB::empty();

        if (_faces.empty())
            return;

        _faceIdx.resize(_faces.size());
        std::iota(_faceIdx.begin(), _faceIdx.end(), 0);
        _nodes.reserve(2 * _faces.size());
        buildRecursive(0, static_cast<int>(_faces.size()));
    }

    void addFace(const std::string &t0, const std::string &t1, const std::string &t2,
        const std::vector<Vec3> &verts, const std::vector<Vec3> &norms) {
        int vi[3], ni[3];
        parseFaceToken(t0, vi[0], ni[0]);
        parseFaceToken(t1, vi[1], ni[1]);
        parseFaceToken(t2, vi[2], ni[2]);

        MeshFace f;
        f.v0 = verts[resolveIdx(vi[0], static_cast<int>(verts.size()))];
        f.v1 = verts[resolveIdx(vi[1], static_cast<int>(verts.size()))];
        f.v2 = verts[resolveIdx(vi[2], static_cast<int>(verts.size()))];
        f.edge1 = f.v1 - f.v0;
        f.edge2 = f.v2 - f.v0;
        f.faceNormal = normalize(cross(f.edge1, f.edge2));

        bool hasNormals = ni[0] != 0 && ni[1] != 0 && ni[2] != 0 && !norms.empty();
        if (hasNormals) {
            f.n0 = normalize(norms[resolveIdx(ni[0], static_cast<int>(norms.size()))]);
            f.n1 = normalize(norms[resolveIdx(ni[1], static_cast<int>(norms.size()))]);
            f.n2 = normalize(norms[resolveIdx(ni[2], static_cast<int>(norms.size()))]);
        } else
            f.n0 = f.n1 = f.n2 = f.faceNormal;

        constexpr double pad = 1e-4;
        f.bbox = AABB(
            Vec3(std::min({f.v0.x, f.v1.x, f.v2.x}),
                  std::min({f.v0.y, f.v1.y, f.v2.y}),
                  std::min({f.v0.z, f.v1.z, f.v2.z}) - pad),
            Vec3(std::max({f.v0.x, f.v1.x, f.v2.x}),
                  std::max({f.v0.y, f.v1.y, f.v2.y}),
                  std::max({f.v0.z, f.v1.z, f.v2.z}) + pad)
        );
        _globalBox = AABB::merge(_globalBox, f.bbox);
        _faces.push_back(f);
    }

    int buildRecursive(int begin, int end) {
        int nodeIdx = static_cast<int>(_nodes.size());
        _nodes.push_back({});

        AABB box = AABB::empty();
        for (int i=begin; i<end; i++)
            box = AABB::merge(box, _faces[_faceIdx[i]].bbox);
        _nodes[nodeIdx].box = box;
        _nodes[nodeIdx].faceBegin = begin;
        _nodes[nodeIdx].faceEnd = end;

        if (end - begin <= MAX_LEAF_SIZE)
            return nodeIdx;

        struct Bin {
            AABB box = AABB::empty();
            int count = 0;
        };

        const double parentSA = surfaceArea(box);
        double bestCost = static_cast<double>(end - begin);
        int bestAxis = -1;
        int bestBin = -1;
        double bestCMin = 0.0, bestCMax = 0.0;

        if (parentSA > 0.0) {
            for (int axis = 0; axis < 3; axis++) {
                double cMin = std::numeric_limits<double>::infinity();
                double cMax = -std::numeric_limits<double>::infinity();
                for (int i = begin; i < end; i++) {
                    Vec3 c = _faces[_faceIdx[i]].bbox.centroid();
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    if (coord < cMin)
                        cMin = coord;
                    if (coord > cMax)
                        cMax = coord;
                }
                if (cMax <= cMin) continue;

                Bin bins[NUM_BINS];
                double invRange = static_cast<double>(NUM_BINS) / (cMax - cMin);
                for (int i = begin; i < end; i++) {
                    Vec3 c = _faces[_faceIdx[i]].bbox.centroid();
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    int b = static_cast<int>((coord - cMin) * invRange);
                    if (b >= NUM_BINS)
                        b = NUM_BINS - 1;
                    bins[b].box = AABB::merge(bins[b].box, _faces[_faceIdx[i]].bbox);
                    bins[b].count++;
                }

                AABB leftBox[NUM_BINS - 1];
                int leftCnt[NUM_BINS - 1];
                {
                    AABB lb = AABB::empty(); int lc = 0;
                    for (int b = 0; b < NUM_BINS - 1; b++) {
                        lb = AABB::merge(lb, bins[b].box);
                        lc += bins[b].count;
                        leftBox[b] = lb;
                        leftCnt[b] = lc;
                    }
                }

                AABB rightBox[NUM_BINS - 1];
                int rightCnt[NUM_BINS - 1];
                {
                    AABB rb = AABB::empty(); int rc = 0;
                    for (int b = NUM_BINS - 1; b >= 1; b--) {
                        rb = AABB::merge(rb, bins[b].box);
                        rc += bins[b].count;
                        rightBox[b - 1] = rb;
                        rightCnt[b - 1] = rc;
                    }
                }

                for (int b = 0; b < NUM_BINS - 1; b++) {
                    if (leftCnt[b] == 0 || rightCnt[b] == 0) continue;
                    double cost = (surfaceArea(leftBox[b]) * leftCnt[b] + surfaceArea(rightBox[b]) * rightCnt[b]) / parentSA;
                    if (cost < bestCost) {
                        bestCost = cost;
                        bestAxis = axis;
                        bestBin = b;
                        bestCMin = cMin;
                        bestCMax = cMax;
                    }
                }
            }
        }

        int midPos;

        if (bestAxis != -1) {
            double invRange = static_cast<double>(NUM_BINS) / (bestCMax - bestCMin);
            auto midIt = std::partition(
                _faceIdx.begin() + begin,
                _faceIdx.begin() + end,
                [&](int idx) {
                    Vec3 c = _faces[idx].bbox.centroid();
                    double coord = (bestAxis == 0) ? c.x : (bestAxis == 1 ? c.y : c.z);
                    int b = static_cast<int>((coord - bestCMin) * invRange);
                    if (b >= NUM_BINS) b = NUM_BINS - 1;
                    return b <= bestBin;
                }
            );
            midPos = static_cast<int>(midIt - _faceIdx.begin());
        } else {
            Vec3 ext = box.max - box.min;
            int axis = 0;
            if (ext.y > ext.x) axis = 1;
            if (ext.z > (axis == 0 ? ext.x : ext.y)) axis = 2;
            double mid = axis == 0 ? (box.min.x + box.max.x) * 0.5
                       : axis == 1 ? (box.min.y + box.max.y) * 0.5
                       : (box.min.z + box.max.z) * 0.5;
            auto midIt = std::partition(
                _faceIdx.begin() + begin,
                _faceIdx.begin() + end,
                [&](int idx) {
                    Vec3 c = _faces[idx].bbox.centroid();
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    return coord < mid;
                }
            );
            midPos = static_cast<int>(midIt - _faceIdx.begin());
        }

        if (midPos == begin || midPos == end)
            midPos = begin + (end - begin) / 2;

        int leftIdx = buildRecursive(begin, midPos);
        int rightIdx = buildRecursive(midPos, end);
        _nodes[nodeIdx].left = leftIdx;
        _nodes[nodeIdx].right = rightIdx;
        return nodeIdx;
    }

    std::optional<HitRecord> traverseNode(int nodeIdx, const Ray &ray, double tMin, double &tMax) const {
        double localMin = tMin;
        double localMax = tMax;
        if (!_nodes[nodeIdx].box.intersect(ray, localMin, localMax))
            return std::nullopt;

        const MeshNode &node = _nodes[nodeIdx];

        if (node.left == -1) {
            std::optional<HitRecord> closest;
            for (int i=node.faceBegin; i<node.faceEnd; i++) {
                auto hit = intersectFace(_faces[_faceIdx[i]], ray);
                if (hit && hit->t > tMin && hit->t <= tMax) {
                    tMax = hit->t;
                    closest = hit;
                }
            }
            return closest;
        }

        auto leftHit = traverseNode(node.left, ray, tMin, tMax);
        auto rightHit = traverseNode(node.right, ray, tMin, tMax);
        return rightHit ? rightHit : leftHit;
    }

    std::optional<HitRecord> intersectFace(const MeshFace &face, const Ray &ray) const {
        Vec3 pvec = cross(ray.direction, face.edge2);
        double det = dot(face.edge1, pvec);
        if (std::fabs(det) < epsilon)
            return std::nullopt;

        double invDet = 1.0 / det;
        Vec3 tvec = ray.origin - face.v0;
        double u = dot(tvec, pvec) * invDet;
        if (u < 0.0 || u > 1.0)
            return std::nullopt;

        Vec3 qvec = cross(tvec, face.edge1);
        double v = dot(ray.direction, qvec) * invDet;
        if (v < 0.0 || u + v > 1.0)
            return std::nullopt;

        double t = dot(face.edge2, qvec) * invDet;
        if (t < epsilon)
            return std::nullopt;

        Vec3 point = ray.at(t);
        double w = 1.0 - u - v;
        Vec3 normal = normalize(face.n0 * w + face.n1 * u + face.n2 * v);
        bool front = dot(ray.direction, normal) < 0.0;
        if (!front)
            normal = -normal;

        Vec3 tangent = normalize(face.edge1);
        Vec3 bitangent = normalize(cross(normal, tangent));

        return HitRecord{t, point, normal, _material, front, {u, v}, tangent, bitangent};
    }
};

extern "C" IPrimitive *create() { return new ObjMesh(); }
extern "C" void destroy(IPrimitive *p) { delete p; }
