/*
 * Project: Raytracer
 * File name: BVH.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: BVH build (midpoint split along longest centroid axis) and traversal.
 */

#include <algorithm>
#include <limits>
#include "BVH.hpp"
#include "IPrimitive.hpp"

static constexpr int MAX_LEAF_SIZE = 4;

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

    AABB centroidBounds = AABB::empty();
    for (int i = begin; i < end; i++) {
        Vec3 c = prims[_primIndices[i]]->boundingBox().centroid();
        centroidBounds = AABB::merge(centroidBounds, AABB(c, c));
    }
    Vec3 ext = centroidBounds.max - centroidBounds.min;
    int axis = 0;
    if (ext.y > ext.x) axis = 1;
    if (ext.z > (axis == 0 ? ext.x : ext.y)) axis = 2;

    double mid = axis == 0 ? (centroidBounds.min.x + centroidBounds.max.x) * 0.5
               : axis == 1 ? (centroidBounds.min.y + centroidBounds.max.y) * 0.5
               : (centroidBounds.min.z + centroidBounds.max.z) * 0.5;

    auto midIt = std::partition(
        _primIndices.begin() + begin,
        _primIndices.begin() + end,
        [&](int idx) {
            Vec3 c = prims[idx]->boundingBox().centroid();
            double coord = (axis == 0) ? c.x : (axis == 1 ? c.y : c.z);
            return coord < mid;
        }
    );

    int midPos = static_cast<int>(midIt - _primIndices.begin());
    if (midPos == begin || midPos == end)
        midPos = begin + count / 2;

    int leftChild  = buildRecursive(prims, begin, midPos);
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

    if (!node.box.intersect(ray, tMin, tMax))
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
