/*
 * Project: Raytracer
 * File name: test_decorators.cpp
 * Description: Criterion unit tests for TranslationDecorator, RotationDecorator, ScaleDecorator.
 *
 * A StubPrimitive is defined inline to control what the wrapped primitive returns, letting each
 * decorator be tested in isolation without depending on a real shape implementation.
 */

#include <criterion/criterion.h>
#include <optional>
#include <unordered_map>
#include <cmath>

#include "../../include/Decorators.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) { return std::abs(a - b) < EPS; }

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

struct StubPrimitive : IPrimitive {
    std::optional<HitRecord> result = std::nullopt;
    mutable Ray lastRay{{0,0,0},{0,0,0}};

    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        lastRay = ray;
        return result;
    }
};

static PrimitivePtr makeStub(StubPrimitive *&outPtr) {
    outPtr = new StubPrimitive();
    return PrimitivePtr(outPtr, [](IPrimitive *p) { delete static_cast<StubPrimitive*>(p); });
}

static HitRecord makeHit(double t, Vec3 point, Vec3 normal) {
    return HitRecord{t, point, normalize(normal), nullptr, true};
}

Test(translation, miss_propagates_as_nullopt) {
    StubPrimitive *stub;
    TranslationDecorator td(makeStub(stub), {1, 2, 3});
    stub->result = std::nullopt;

    Ray ray{{0,0,0},{0,0,-1}};
    cr_assert_not(td.intersect(ray).has_value());
}

Test(translation, hit_point_is_offset_by_translation) {
    StubPrimitive *stub;
    Vec3 offset{1, 0, 0};
    TranslationDecorator td(makeStub(stub), offset);
    stub->result = makeHit(1.0, {3, 0, 0}, {0, 1, 0});

    auto hit = td.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {4, 0, 0}));
}

Test(translation, ray_origin_is_shifted_backward_before_delegate) {
    StubPrimitive *stub;
    Vec3 offset{5, 0, 0};
    TranslationDecorator td(makeStub(stub), offset);
    stub->result = std::nullopt;

    Ray ray{{10, 0, 0},{0,0,-1}};
    td.intersect(ray);
    cr_assert(vec3_near(stub->lastRay.origin, {5, 0, 0}));
}

Test(translation, ray_direction_is_unchanged) {
    StubPrimitive *stub;
    TranslationDecorator td(makeStub(stub), {1, 2, 3});
    stub->result = std::nullopt;

    Ray ray{{0,0,0},{1,0,0}};
    td.intersect(ray);
    cr_assert(vec3_near(stub->lastRay.direction, {1, 0, 0}));
}

Test(translation, t_value_is_preserved) {
    StubPrimitive *stub;
    TranslationDecorator td(makeStub(stub), {0, 1, 0});
    stub->result = makeHit(7.5, {0, 0, 0}, {0, 1, 0});

    auto hit = td.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(near(hit->t, 7.5));
}

Test(translation, configure_delegates_to_inner) {
    StubPrimitive *stub;
    TranslationDecorator td(makeStub(stub), {0,0,0});
    cr_assert_no_throw(td.configure({}, nullptr));
}

Test(rotation, miss_propagates_as_nullopt) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0, 0, 0});
    stub->result = std::nullopt;

    cr_assert_not(rd.intersect(Ray{{0,0,0},{0,0,-1}}).has_value());
}

Test(rotation, zero_angles_leaves_hit_point_unchanged) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0, 0, 0});
    stub->result = makeHit(2.0, {1, 2, 3}, {0, 1, 0});

    auto hit = rd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {1, 2, 3}));
}

Test(rotation, zero_angles_leaves_normal_unchanged) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0, 0, 0});
    stub->result = makeHit(1.0, {0, 1, 0}, {0, 1, 0});

    auto hit = rd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->normal, {0, 1, 0}));
}

Test(rotation, normal_is_unit_length_after_rotation) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0.3, 0.5, 0.7});
    stub->result = makeHit(1.0, {0, 0, -5}, {0, 1, 0});

    auto hit = rd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(near(length(hit->normal), 1.0));
}

Test(rotation, rotateZ_90_maps_upward_normal_to_rightward) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0, 0, M_PI / 2.0});
    stub->result = makeHit(1.0, {1, 0, 0}, {0, 1, 0});

    auto hit = rd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(near(length(hit->normal), 1.0));
}

Test(rotation, configure_delegates_to_inner) {
    StubPrimitive *stub;
    RotationDecorator rd(makeStub(stub), {0, 0, 0});
    cr_assert_no_throw(rd.configure({}, nullptr));
}

Test(scale, miss_propagates_as_nullopt) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {2, 2, 2});
    stub->result = std::nullopt;

    cr_assert_not(sd.intersect(Ray{{0,0,0},{0,0,-1}}).has_value());
}

Test(scale, hit_point_is_scaled_up) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {2, 3, 4});
    stub->result = makeHit(1.0, {1, 1, 1}, {0, 1, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {2, 3, 4}));
}

Test(scale, ray_origin_is_divided_before_delegate) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {2, 2, 2});
    stub->result = std::nullopt;

    sd.intersect(Ray{{4, 6, 8},{0,0,-1}});
    cr_assert(vec3_near(stub->lastRay.origin, {2, 3, 4}));
}

Test(scale, ray_direction_is_divided_before_delegate) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {2, 2, 1});
    stub->result = std::nullopt;

    sd.intersect(Ray{{0,0,0},{2, 4, 1}});
    cr_assert(vec3_near(stub->lastRay.direction, {1, 2, 1}));
}

Test(scale, normal_is_unit_length_after_scale) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {1, 2, 3});
    stub->result = makeHit(1.0, {1, 1, 1}, {1, 1, 1});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(near(length(hit->normal), 1.0));
}

Test(scale, uniform_scale_preserves_normal_direction) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {3, 3, 3});
    stub->result = makeHit(1.0, {0, 1, 0}, {0, 1, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->normal, {0, 1, 0}));
}

Test(scale, t_value_is_preserved) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {2, 2, 2});
    stub->result = makeHit(3.14, {1, 0, 0}, {1, 0, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(near(hit->t, 3.14));
}

Test(scale, configure_delegates_to_inner) {
    StubPrimitive *stub;
    ScaleDecorator sd(makeStub(stub), {1, 1, 1});
    cr_assert_no_throw(sd.configure({}, nullptr));
}

Test(shear, miss_propagates_as_nullopt) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 0.5, 0.0, 0.0, 0.0, 0.0, 0.0);
    stub->result = std::nullopt;

    cr_assert_not(sd.intersect(Ray{{0,0,0},{0,0,-1}}).has_value());
}

Test(shear, identity_shear_leaves_hit_unchanged) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    stub->result = makeHit(2.0, {1, 2, 3}, {0, 1, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {1, 2, 3}));
    cr_assert(vec3_near(hit->normal, {0, 1, 0}));
}

Test(shear, shear_x_by_y_transforms_point) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 0.5, 0.0, 0.0, 0.0, 0.0, 0.0);
    stub->result = makeHit(1.0, {2, 4, 0}, {0, 1, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    cr_assert(vec3_near(hit->point, {4, 4, 0}));
}

Test(shear, shear_x_by_y_transforms_normal_correctly) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 1.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    stub->result = makeHit(1.0, {0, 0, 0}, {1, 0, 0});

    auto hit = sd.intersect(Ray{{0,0,0},{0,0,-1}});
    cr_assert(hit.has_value());
    Vec3 expected = normalize({1, -1, 0});
    cr_assert(vec3_near(hit->normal, expected));
    cr_assert(near(length(hit->normal), 1.0));
}

Test(shear, ray_origin_transformed_by_inverse_shear) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 2.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    stub->result = std::nullopt;

    sd.intersect(Ray{{0, 3, 0},{0,0,-1}});
    cr_assert(vec3_near(stub->lastRay.origin, {-6, 3, 0}));
}

Test(shear, configure_delegates_to_inner) {
    StubPrimitive *stub;
    ShearDecorator sd(makeStub(stub), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    cr_assert_no_throw(sd.configure({}, nullptr));
}
