/*
 * Project: Raytracer
 * File name: test_bvh.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for BVH - build and intersect.
 */

#include <criterion/criterion.h>
#include <memory>
#include <unordered_map>
#include "BVH.hpp"
#include "IPrimitive.hpp"
#include "AABB.hpp"
#include "Ray.hpp"
#include "Vec3.hpp"
#include "HitRecord.hpp"
#include "IMaterial.hpp"

class MockBoundedPrimitive : public IPrimitive {
public:
    AABB box;
    double hitT;
    Vec3 hitNormal;

    MockBoundedPrimitive(AABB b, double t = 1.0, Vec3 n = {0, 0, 1}) : box(b), hitT(t), hitNormal(n) {}

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        double tMin = 0, tMax = hitT;
        if (box.intersect(ray, tMin, tMax)) {
            HitRecord rec;
            rec.t = hitT;
            rec.point = ray.at(hitT);
            rec.normal = hitNormal;
            rec.frontFace = true;
            return rec;
        }
        return std::nullopt;
    }

    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}

    AABB boundingBox() const override { return box; }
};

class MockUnboundedPrimitive : public IPrimitive {
public:
    double hitT;
    Vec3 hitNormal;

    MockUnboundedPrimitive(double t = 1.0, Vec3 n = {0, 0, 1}) : hitT(t), hitNormal(n) {}

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        HitRecord rec;
        rec.t = hitT;
        rec.point = ray.at(hitT);
        rec.normal = hitNormal;
        rec.frontFace = true;
        return rec;
    }

    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}

    AABB boundingBox() const override { return AABB::infinite(); }
};

static PrimitivePtr makeBoundedPrimitive(AABB box, double t = 1.0, Vec3 n = {0, 0, 1}) {
    return PrimitivePtr(new MockBoundedPrimitive(box, t, n), [](IPrimitive *p) { delete p; });
}

static PrimitivePtr makeUnboundedPrimitive(double t = 1.0, Vec3 n = {0, 0, 1}) {
    return PrimitivePtr(new MockUnboundedPrimitive(t, n), [](IPrimitive *p) { delete p; });
}

Test(bvh, build_empty) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    bvh.build(primitives);
    Ray ray(Vec3(0,0,0), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(!hit.has_value());
}

Test(bvh, build_bounded_only) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1)), 1.5));
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(2,2,2), Vec3(3,3,3)), 4.5));
    bvh.build(primitives);
    Ray ray(Vec3(0.5,0.5,-1), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 1.5, 1e-15);
}

Test(bvh, build_unbounded_only) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeUnboundedPrimitive(1.0));
    primitives.push_back(makeUnboundedPrimitive(2.0));
    bvh.build(primitives);
    Ray ray(Vec3(0,0,0), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 1.0, 1e-15);
}

Test(bvh, build_mixed) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1)), 1.0));
    primitives.push_back(makeUnboundedPrimitive(0.5));
    bvh.build(primitives);
    Ray ray(Vec3(0.5,0.5,-1), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 0.5, 1e-15);
}

Test(bvh, intersect_no_hit) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1))));
    bvh.build(primitives);
    Ray ray(Vec3(10,10,10), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(!hit.has_value());
}

Test(bvh, intersect_hit_bounded) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1)), 1.5));
    bvh.build(primitives);
    Ray ray(Vec3(0.5,0.5,-1), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 1.5, 1e-15);
}

Test(bvh, intersect_hit_unbounded) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeUnboundedPrimitive(0.5));
    bvh.build(primitives);
    Ray ray(Vec3(0,0,-1), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 0.5, 1e-15);
}

Test(bvh, intersect_closest_hit) {
    BVH bvh;
    std::vector<PrimitivePtr> primitives;
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1)), 1.5));
    primitives.push_back(makeBoundedPrimitive(AABB(Vec3(0,0,0), Vec3(1,1,1)), 1.2));
    bvh.build(primitives);
    Ray ray(Vec3(0.5,0.5,-1), Vec3(0,0,1));
    auto hit = bvh.intersect(ray, primitives);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 1.2, 1e-15);
}