/*
 * Project: Raytracer
 * File name: GroupNode.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene graph group node implementation.
 */

#include "GroupNode.hpp"
#include "Mat3.hpp"
#include "Common.hpp"
#include <limits>

constexpr int NB_CORNERS_AABB = 8;

GroupNode::GroupNode(std::vector<PrimitivePtr> children)
    : _children(std::move(children)) {}

GroupNode::GroupNode(std::vector<PrimitivePtr> children, const Mat4 &localTransform)
    : _children(std::move(children)),
      _transform(localTransform),
      _invTransform(localTransform.inverse()),
      _hasTransform(true) {}

void GroupNode::addChild(PrimitivePtr child) {
    _children.push_back(std::move(child));
}

void GroupNode::setTransform(const Mat4 &localTransform) {
    _transform = localTransform;
    _invTransform = localTransform.inverse();
    _hasTransform = true;
}

void GroupNode::configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) {}

AABB GroupNode::boundingBox() const {
    AABB localBounds = AABB::empty();
    for (const auto &child : _children)
        localBounds = AABB::merge(localBounds, child->boundingBox());

    if (!_hasTransform || localBounds.isInfinite())
        return localBounds;

    const Vec3 mn = localBounds.min, mx = localBounds.max;
    const Vec3 corners[NB_CORNERS_AABB] = {
        {mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z},
        {mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z},
        {mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z},
        {mn.x, mx.y, mx.z}, {mx.x, mx.y, mx.z}
    };

    AABB worldBounds = AABB::empty();
    for (const Vec3 &c : corners) {
        Vec3 wc = transformPoint(_transform, c);
        worldBounds = AABB::merge(worldBounds, AABB(wc, wc));
    }
    return worldBounds;
}

std::optional<HitRecord> GroupNode::intersect(const Ray &ray) const {
    Ray localRay = _hasTransform
        ? Ray{transformPoint(_invTransform, ray.origin),
              transformDirection(_invTransform, ray.direction)}
        : ray;

    std::optional<HitRecord> closest;
    double tMax = std::numeric_limits<double>::infinity();

    for (const auto &child : _children) {
        auto hit = child->intersect(localRay);
        if (hit && hit->t > epsilon && hit->t < tMax) {
            tMax = hit->t;
            closest = hit;
        }
    }

    if (closest && _hasTransform) {
        closest->point = transformPoint(_transform, closest->point);

        Mat3 inv3x3 = {{
            {_invTransform.m[0][0], _invTransform.m[0][1], _invTransform.m[0][2]},
            {_invTransform.m[1][0], _invTransform.m[1][1], _invTransform.m[1][2]},
            {_invTransform.m[2][0], _invTransform.m[2][1], _invTransform.m[2][2]}
        }};
        closest->normal = normalize(inv3x3.transpose() * closest->normal);
    }

    return closest;
}

PrimitivePtr makeGroup() {
    return PrimitivePtr(new GroupNode(), &defaultDestroy<GroupNode>);
}

PrimitivePtr makeGroup(std::vector<PrimitivePtr> children) {
    return PrimitivePtr(new GroupNode(std::move(children)), &defaultDestroy<GroupNode>);
}

PrimitivePtr makeGroup(std::vector<PrimitivePtr> children, const Mat4 &localTransform) {
    return PrimitivePtr(new GroupNode(std::move(children), localTransform), &defaultDestroy<GroupNode>);
}
