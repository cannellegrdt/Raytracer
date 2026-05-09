/*
 * Project: Raytracer
 * File name: test_groupnode.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for GroupNode.
 */

#include <criterion/criterion.h>
#include <optional>
#include <unordered_map>
#include <cmath>
#include <memory>

#include "GroupNode.hpp"
#include "IPrimitive.hpp"
#include "Mat4.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) { return std::abs(a - b) < EPS; }

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

class StubPrimitive : public IPrimitive {
public:
    std::optional<HitRecord> result = std::nullopt;
    mutable Ray lastRay{{0,0,0},{0,0,0}};
    bool returnInfinite = false;

    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        lastRay = ray;
        return result;
    }
    AABB boundingBox() const override { return returnInfinite ? AABB::infinite() : AABB(Vec3(0,0,0), Vec3(1,1,1)); }
};

static PrimitivePtr makeStub(StubPrimitive *&outPtr) {
    outPtr = new StubPrimitive();
    return PrimitivePtr(outPtr, [](IPrimitive *p) { delete static_cast<StubPrimitive*>(p); });
}

static HitRecord makeHit(double t, Vec3 point, Vec3 normal) {
    return HitRecord{t, point, normalize(normal), nullptr, true, {0.0, 0.0}, Vec3(0,0,0), Vec3(0,0,0)};
}

Test(groupnode, default_constructor_creates_empty_group) {
    GroupNode node;
    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(groupnode, constructor_with_children_stores_them) {
    StubPrimitive *stub1;
    StubPrimitive *stub2;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub1));
    children.push_back(makeStub(stub2));

    GroupNode node(std::move(children));
    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(groupnode, intersect_returns_nullopt_when_all_children_miss) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = std::nullopt;

    GroupNode node(std::move(children));
    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(groupnode, intersect_returns_closest_hit) {
    StubPrimitive *stub1;
    StubPrimitive *stub2;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub1));
    children.push_back(makeStub(stub2));

    stub1->result = makeHit(5.0, {0, 0, -5}, {0, 1, 0});
    stub2->result = makeHit(3.0, {0, 0, -3}, {0, 1, 0});

    GroupNode node(std::move(children));
    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(near(hit->t, 3.0));
}

Test(groupnode, intersect_transforms_ray_when_transform_set) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = std::nullopt;

    Mat4 transform = translate(0.0, 0.0, -10.0);
    GroupNode node(std::move(children), transform);

    Ray ray{{0,0,0},{0,0,-1}};
    node.intersect(ray);
    cr_assert(vec3_near(stub->lastRay.origin, {0, 0, 10}));
}

Test(groupnode, intersect_transforms_hit_point_back_to_world) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = makeHit(1.0, {0, 0, -5}, {0, 1, 0});

    Mat4 transform = translate(0.0, 0.0, -10.0);
    GroupNode node(std::move(children), transform);

    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {0, 0, -15}));
}

Test(groupnode, intersect_without_transform_does_not_modify_ray) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = std::nullopt;

    GroupNode node(std::move(children));

    Ray ray{{1, 2, 3},{0, 0, -1}};
    node.intersect(ray);
    cr_assert(vec3_near(stub->lastRay.origin, {1, 2, 3}));
}

Test(groupnode, addChild_appends_to_group) {
    GroupNode node;
    StubPrimitive *stub1;
    StubPrimitive *stub2;

    node.addChild(makeStub(stub1));
    node.addChild(makeStub(stub2));

    stub1->result = makeHit(2.0, {0, 0, -2}, {0, 1, 0});
    stub2->result = std::nullopt;

    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(near(hit->t, 2.0));
}

Test(groupnode, setTransform_updates_transform) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = std::nullopt;

    GroupNode node(std::move(children));
    Mat4 transform = translate(5.0, 0.0, 0.0);
    node.setTransform(transform);

    Ray ray{{5,0,0},{0,0,-1}};
    node.intersect(ray);
    cr_assert(vec3_near(stub->lastRay.origin, {0, 0, 0}));
}

Test(groupnode, configure_is_noop) {
    GroupNode node;
    cr_assert_no_throw(node.configure({}, nullptr));
}

Test(groupnode, makeGroup_creates_empty_group) {
    auto group = makeGroup();
    Ray ray{{0,0,0},{0,0,-1}};
    cr_assert_not(group->intersect(ray).has_value());
}

Test(groupnode, makeGroup_with_children) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = makeHit(1.0, {0, 0, -1}, {0, 1, 0});

    auto group = makeGroup(std::move(children));
    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = group->intersect(ray);
    cr_assert(hit.has_value());
}

Test(groupnode, makeGroup_with_children_and_transform) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->result = makeHit(1.0, {0, 0, 0}, {0, 1, 0});

    Mat4 transform = translate(0.0, 0.0, -10.0);
    auto group = makeGroup(std::move(children), transform);

    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = group->intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {0, 0, -10}));
}

Test(groupnode, nested_groups_work) {
    StubPrimitive *innerStub;
    std::vector<PrimitivePtr> innerChildren;
    innerChildren.push_back(makeStub(innerStub));
    innerStub->result = makeHit(1.0, {0, 0, 0}, {0, 1, 0});

    auto innerGroup = makeGroup(std::move(innerChildren), translate(0.0, 0.0, -5.0));

    std::vector<PrimitivePtr> outerChildren;
    outerChildren.push_back(std::move(innerGroup));
    auto outerGroup = makeGroup(std::move(outerChildren), translate(0.0, 0.0, -5.0));

    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = outerGroup->intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {0, 0, -10}));
}

Test(groupnode, normal_is_transformed_correctly) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));

    Vec3 originalNormal{0, 1, 0};
    stub->result = makeHit(1.0, {0, 0, 0}, originalNormal);

    Mat4 transform = scale(1.0, 2.0, 1.0);
    GroupNode node(std::move(children), transform);

    Ray ray{{0,0,0},{0,0,-1}};
    auto hit = node.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(near(length(hit->normal), 1.0));
}

Test(groupnode, boundingBox_with_infinite_child_returns_infinite) {
    StubPrimitive *stub;
    std::vector<PrimitivePtr> children;
    children.push_back(makeStub(stub));
    stub->returnInfinite = true;

    GroupNode node(std::move(children));
    AABB box = node.boundingBox();
    cr_assert(box.isInfinite());
}
