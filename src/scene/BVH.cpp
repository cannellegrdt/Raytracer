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
#include <omp.h>
#include "BVH.hpp"
#include "IPrimitive.hpp"
#include "SAHBuilder.hpp"
#include "utils/Common.hpp"

constexpr int MAX_DEPTH_TREE = 64;

void BVH::build(const std::vector<PrimitivePtr> &primitives) {
    _nodes.clear();
    _primIndices.clear();
    _unbounded.clear();

    std::vector<AABB> boxes(primitives.size());
    for (int i = 0; i < static_cast<int>(primitives.size()); i++) {
        boxes[i] = primitives[i]->boundingBox();
        if (boxes[i].isInfinite())
            _unbounded.push_back(i);
        else
            _primIndices.push_back(i);
    }

    if (!_primIndices.empty()) {
        _nodes.reserve(_primIndices.size() * 2);
        SAHBuilder<Node> builder(
            _primIndices, _nodes,
            [&boxes](int idx) -> AABB { return boxes[idx]; },
            [&boxes](int idx) -> Vec3  { return boxes[idx].centroid(); });
        builder.build(0, static_cast<int>(_primIndices.size()));
    }
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
    std::optional<HitRecord> closest;
    double current_tMax = tMax;

    int stack[MAX_DEPTH_TREE];
    int top = 0;
    stack[top++] = nodeIdx;

    while (top > 0) {
        const Node &node = _nodes[stack[--top]];

        double localMin = tMin;
        double localMax = current_tMax;
        if (!node.box.intersect(ray, localMin, localMax))
            continue;

        if (node.left == -1) {
            for (int i = node.begin; i < node.end; i++) {
                auto hit = prims[_primIndices[i]]->intersect(ray);
                if (hit && hit->t > tMin && hit->t < current_tMax) {
                    current_tMax = hit->t;
                    closest = hit;
                }
            }
        } else {
            stack[top++] = node.right;
            stack[top++] = node.left;
        }
    }

    tMax = current_tMax;
    return closest;
}
