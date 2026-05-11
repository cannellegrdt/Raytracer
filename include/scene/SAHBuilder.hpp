/*
 * Project: Raytracer
 * File name: SAHBuilder.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Shared SAH-binned BVH construction algorithm. Templated over
 *                   the node type so it can be reused for scene-level primitives
 *                   (BVH.cpp) and mesh-level triangles (obj.cpp).
 */

#ifndef SAHBUILDER_HPP_
    #define SAHBUILDER_HPP_
    #include <algorithm>
    #include <functional>
    #include <limits>
    #include "AABB.hpp"

    static constexpr int NUM_BINS = 12;
    static constexpr int MAX_LEAF_SIZE = 4;

inline double surfaceArea(const AABB &box) {
    Vec3 d = box.max - box.min;
    if (d.x < 0.0 || d.y < 0.0 || d.z < 0.0) return 0.0;
    return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

template <typename Node>
class SAHBuilder {
public:
    using GetAABB = std::function<AABB(int)>;
    using GetCentroid = std::function<Vec3(int)>;

    SAHBuilder(std::vector<int> &indices, std::vector<Node> &nodes,
               GetAABB getAABB, GetCentroid getCentroid)
        : _indices(indices), _nodes(nodes),
          _getAABB(std::move(getAABB)), _getCentroid(std::move(getCentroid)) {}

    int build(int begin, int end) {
        int nodeIdx = static_cast<int>(_nodes.size());
        _nodes.push_back({});

        AABB box = AABB::empty();
        for (int i = begin; i < end; i++)
            box = AABB::merge(box, _getAABB(_indices[i]));
        _nodes[nodeIdx].box = box;
        _nodes[nodeIdx].begin = begin;
        _nodes[nodeIdx].end = end;

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
                    Vec3 c = _getCentroid(_indices[i]);
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    if (coord < cMin) cMin = coord;
                    if (coord > cMax) cMax = coord;
                }
                if (cMax <= cMin) continue;

                Bin bins[NUM_BINS];
                double invRange = static_cast<double>(NUM_BINS) / (cMax - cMin);
                for (int i = begin; i < end; i++) {
                    Vec3 c = _getCentroid(_indices[i]);
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    int b = static_cast<int>((coord - cMin) * invRange);
                    if (b >= NUM_BINS) b = NUM_BINS - 1;
                    bins[b].box = AABB::merge(bins[b].box, _getAABB(_indices[i]));
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
                _indices.begin() + begin, _indices.begin() + end,
                [&](int idx) {
                    Vec3 c = _getCentroid(idx);
                    double coord = (bestAxis == 0) ? c.x : (bestAxis == 1 ? c.y : c.z);
                    int b = static_cast<int>((coord - bestCMin) * invRange);
                    if (b >= NUM_BINS) b = NUM_BINS - 1;
                    return b <= bestBin;
                });
            midPos = static_cast<int>(midIt - _indices.begin());
        } else {
            Vec3 ext = box.max - box.min;
            int axis = 0;
            if (ext.y > ext.x) axis = 1;
            if (ext.z > (axis == 0 ? ext.x : ext.y)) axis = 2;
            double mid = axis == 0 ? (box.min.x + box.max.x) * 0.5
                       : axis == 1 ? (box.min.y + box.max.y) * 0.5
                       : (box.min.z + box.max.z) * 0.5;
            auto midIt = std::partition(
                _indices.begin() + begin, _indices.begin() + end,
                [&](int idx) {
                    Vec3 c = _getCentroid(idx);
                    double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
                    return coord < mid;
                });
            midPos = static_cast<int>(midIt - _indices.begin());
        }

        if (midPos == begin || midPos == end)
            midPos = begin + (end - begin) / 2;

        int leftChild = build(begin, midPos);
        int rightChild = build(midPos, end);
        _nodes[nodeIdx].left = leftChild;
        _nodes[nodeIdx].right = rightChild;
        return nodeIdx;
    }

private:
    std::vector<int> &_indices;
    std::vector<Node> &_nodes;
    GetAABB _getAABB;
    GetCentroid _getCentroid;
};

#endif /* SAHBUILDER_HPP_ */
