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
#include "SAHBuilder.hpp"
#include "Vec3.hpp"

namespace {

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
    int begin = 0;
    int end = 0;
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
    if (size <= 0) return -1;
    int resolved = idx > 0 ? idx - 1 : size + idx;
    if (resolved < 0 || resolved >= size) return -1;
    return resolved;
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
        SAHBuilder<MeshNode> builder(
            _faceIdx, _nodes,
            [this](int idx) -> AABB { return _faces[idx].bbox; },
            [this](int idx) -> Vec3 { return _faces[idx].bbox.centroid(); });
        builder.build(0, static_cast<int>(_faces.size()));
    }

    void addFace(const std::string &t0, const std::string &t1, const std::string &t2,
        const std::vector<Vec3> &verts, const std::vector<Vec3> &norms) {
        int vi[3], ni[3];
        parseFaceToken(t0, vi[0], ni[0]);
        parseFaceToken(t1, vi[1], ni[1]);
        parseFaceToken(t2, vi[2], ni[2]);

        int vri[3];
        vri[0] = resolveIdx(vi[0], static_cast<int>(verts.size()));
        vri[1] = resolveIdx(vi[1], static_cast<int>(verts.size()));
        vri[2] = resolveIdx(vi[2], static_cast<int>(verts.size()));
        if (vri[0] < 0 || vri[1] < 0 || vri[2] < 0)
            return;

        MeshFace f;
        f.v0 = verts[vri[0]];
        f.v1 = verts[vri[1]];
        f.v2 = verts[vri[2]];
        f.edge1 = f.v1 - f.v0;
        f.edge2 = f.v2 - f.v0;
        f.faceNormal = normalize(cross(f.edge1, f.edge2));

        bool hasNormals = ni[0] != 0 && ni[1] != 0 && ni[2] != 0 && !norms.empty();
        if (hasNormals) {
            int nri[3];
            nri[0] = resolveIdx(ni[0], static_cast<int>(norms.size()));
            nri[1] = resolveIdx(ni[1], static_cast<int>(norms.size()));
            nri[2] = resolveIdx(ni[2], static_cast<int>(norms.size()));
            if (nri[0] < 0 || nri[1] < 0 || nri[2] < 0)
                hasNormals = false;
            else {
                f.n0 = normalize(norms[nri[0]]);
                f.n1 = normalize(norms[nri[1]]);
                f.n2 = normalize(norms[nri[2]]);
            }
        }
        if (!hasNormals)
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

    std::optional<HitRecord> traverseNode(int nodeIdx, const Ray &ray, double tMin, double &tMax) const {
        double localMin = tMin;
        double localMax = tMax;
        if (!_nodes[nodeIdx].box.intersect(ray, localMin, localMax))
            return std::nullopt;

        const MeshNode &node = _nodes[nodeIdx];

        if (node.left == -1) {
            std::optional<HitRecord> closest;
            for (int i=node.begin; i<node.end; i++) {
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
