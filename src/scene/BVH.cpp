/*
 * Project: Raytracer
 * File name: BVH.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: BVH with SAH binned evaluation for optimal tree quality.
 *                   Each split candidate is scored by (SA(left)*N_left + SA(right)*N_right)/SA(parent).
 *                   Falls back to longest-axis midpoint when all centroid projections coincide.
 */

#include <algorithm>
#include <limits>
#include "BVH.hpp"
#include "IPrimitive.hpp"

static constexpr int NUM_BINS     = 12;
static constexpr int MAX_LEAF_SIZE = 4;

static double surfaceArea(const AABB &box) {
    Vec3 d = box.max - box.min;
    if (d.x < 0.0 || d.y < 0.0 || d.z < 0.0) return 0.0; // empty/degenerate
    return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

void BVH::build(const std::vector<PrimitivePtr> &primitives) {
    _nodes.clear();
    _primIndices.clear();
    _unbounded.clear();

    for (int i = 0; i < static_cast<int>(primitives.size()); i++) {
        if (primitives[i]->boundingBox().isInfinite())
            _unbounded.push_back(i);
        else
            _primIndices.push_back(i);
    }

    if (!_primIndices.empty()) {
        _nodes.reserve(_primIndices.size() * 2);
        buildRecursive(primitives, 0, static_cast<int>(_primIndices.size()));
    }
}

int BVH::buildRecursive(const std::vector<PrimitivePtr> &prims, int begin, int end) {
    int nodeIdx = static_cast<int>(_nodes.size());
    _nodes.push_back(Node{});

    AABB box = AABB::empty();
    for (int i = begin; i < end; i++)
        box = AABB::merge(box, prims[_primIndices[i]]->boundingBox());
    _nodes[nodeIdx].box = box;

    int count = end - begin;
    if (count <= MAX_LEAF_SIZE) {
        _nodes[nodeIdx].primBegin = begin;
        _nodes[nodeIdx].primEnd   = end;
        return nodeIdx;
    }

    struct Bin { AABB box = AABB::empty(); int count = 0; };

    const double parentSA = surfaceArea(box);
    double bestCost = static_cast<double>(count);
    int bestAxis = -1;
    int bestBin = -1;
    double bestCMin = 0.0, bestCMax = 0.0;

    if (parentSA > 0.0) {
        for (int axis = 0; axis < 3; axis++) {
            double cMin =  std::numeric_limits<double>::infinity();
            double cMax = -std::numeric_limits<double>::infinity();
            for (int i = begin; i < end; i++) {
                Vec3 c = prims[_primIndices[i]]->boundingBox().centroid();
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
                Vec3 c = prims[_primIndices[i]]->boundingBox().centroid();
                double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                int b = static_cast<int>((coord - cMin) * invRange);
                if (b >= NUM_BINS)
                    b = NUM_BINS - 1;
                bins[b].box = AABB::merge(bins[b].box, prims[_primIndices[i]]->boundingBox());
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
            _primIndices.begin() + begin,
            _primIndices.begin() + end,
            [&](int idx) {
                Vec3 c = prims[idx]->boundingBox().centroid();
                double coord = (bestAxis == 0) ? c.x : (bestAxis == 1 ? c.y : c.z);
                int b = static_cast<int>((coord - bestCMin) * invRange);
                if (b >= NUM_BINS) b = NUM_BINS - 1;
                return b <= bestBin;
            }
        );
        midPos = static_cast<int>(midIt - _primIndices.begin());
    } else {
        Vec3 ext = box.max - box.min;
        int axis = 0;
        if (ext.y > ext.x) axis = 1;
        if (ext.z > (axis == 0 ? ext.x : ext.y)) axis = 2;
        double mid = axis == 0 ? (box.min.x + box.max.x) * 0.5
                   : axis == 1 ? (box.min.y + box.max.y) * 0.5
                   : (box.min.z + box.max.z) * 0.5;
        auto midIt = std::partition(
            _primIndices.begin() + begin,
            _primIndices.begin() + end,
            [&](int idx) {
                Vec3 c = prims[idx]->boundingBox().centroid();
                double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                return coord < mid;
            }
        );
        midPos = static_cast<int>(midIt - _primIndices.begin());
    }

    if (midPos == begin || midPos == end)
        midPos = begin + count / 2;

    int leftChild = buildRecursive(prims, begin, midPos);
    int rightChild = buildRecursive(prims, midPos, end);
    _nodes[nodeIdx].left = leftChild;
    _nodes[nodeIdx].right = rightChild;
    _nodes[nodeIdx].primBegin = -1;
    _nodes[nodeIdx].primEnd = -1;

    return nodeIdx;
}

std::optional<HitRecord> BVH::intersect(const Ray &ray, const std::vector<PrimitivePtr> &primitives) const {
    double tMax = std::numeric_limits<double>::infinity();
    std::optional<HitRecord> closest;

    if (!_nodes.empty()) {
        auto hit = traverseNode(0, ray, epsilon, tMax, primitives);
        if (hit)
            closest = hit;
    }

    for (int idx : _unbounded) {
        auto hit = primitives[idx]->intersect(ray);
        if (hit && hit->t > epsilon && hit->t < tMax) {
            tMax = hit->t;
            closest = hit;
        }
    }

    return closest;
}

std::optional<HitRecord> BVH::traverseNode(int nodeIdx, const Ray &ray, double tMin, double &tMax,
    const std::vector<PrimitivePtr> &prims) const {
    const Node &node = _nodes[nodeIdx];

    double localMin = tMin;
    double localMax = tMax;
    if (!node.box.intersect(ray, localMin, localMax))
        return std::nullopt;

    if (node.left == -1) {
        std::optional<HitRecord> closest;
        for (int i = node.primBegin; i < node.primEnd; i++) {
            auto hit = prims[_primIndices[i]]->intersect(ray);
            if (hit && hit->t > tMin && hit->t < tMax) {
                tMax = hit->t;
                closest = hit;
            }
        }
        return closest;
    }

    auto leftHit = traverseNode(node.left,  ray, tMin, tMax, prims);
    auto rightHit = traverseNode(node.right, ray, tMin, tMax, prims);
    return rightHit ? rightHit : leftHit;
}
