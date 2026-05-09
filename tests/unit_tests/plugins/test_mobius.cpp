/*
 * Project: Raytracer
 * File name: test_mobius.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Mobius strip plugin - configuration, bounding box, and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>
#include <memory>

#include <interfaces/IMaterial.hpp>

#define create mobius_create_fn
#define destroy mobius_destroy_fn
#include "../../src/plugins/mobius.cpp"
#undef create
#undef destroy

static std::shared_ptr<IMaterial> dummyMaterial() {
    struct DummyMat : public IMaterial {
        ScatterResult scatter(const Ray &, const HitRecord &) const override {
            return {{1.0, 1.0, 1.0}, std::nullopt, std::nullopt};
        }
    };
    return std::shared_ptr<IMaterial>(new DummyMat());
}

static std::unordered_map<std::string, double> mobiusParams(
    double x, double y, double z, double R, double w)
{
    return {{"x", x}, {"y", y}, {"z", z}, {"R", R}, {"w", w}};
}

Test(mobius, plugin_create_returns_non_null) {
    IPrimitive *p = mobius_create_fn();
    cr_assert_not_null(p);
    mobius_destroy_fn(p);
}

Test(mobius, plugin_destroy_deletes_object) {
    IPrimitive *p = mobius_create_fn();
    cr_assert_no_throw(mobius_destroy_fn(p));
}

Test(mobius, configure_with_valid_params_no_throw) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    cr_assert_no_throw(strip.configure(params, dummyMaterial()));
}

Test(mobius, configure_negative_R_throws) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, -1.0, 0.1);
    cr_assert_throw(strip.configure(params, dummyMaterial()), std::invalid_argument);
}

Test(mobius, configure_zero_R_throws) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 0.0, 0.1);
    cr_assert_throw(strip.configure(params, dummyMaterial()), std::invalid_argument);
}

Test(mobius, configure_negative_w_throws) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, -0.1);
    cr_assert_throw(strip.configure(params, dummyMaterial()), std::invalid_argument);
}

Test(mobius, configure_zero_w_throws) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.0);
    cr_assert_throw(strip.configure(params, dummyMaterial()), std::invalid_argument);
}

Test(mobius, bounding_box_valid_after_default_configure) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    AABB box = strip.boundingBox();
    cr_assert(box.min.x < box.max.x);
    cr_assert(box.min.y < box.max.y);
    cr_assert(box.min.z < box.max.z);
    double expected_radius = 1.1;
    cr_assert_float_eq(box.min.x, -expected_radius, 1e-10);
    cr_assert_float_eq(box.min.y, -expected_radius, 1e-10);
    cr_assert_float_eq(box.min.z, -0.1, 1e-10);
    cr_assert_float_eq(box.max.x, expected_radius, 1e-10);
    cr_assert_float_eq(box.max.y, expected_radius, 1e-10);
    cr_assert_float_eq(box.max.z, 0.1, 1e-10);
}

Test(mobius, translated_mobius_bounding_box_correct) {
    MobiusStrip strip;
    auto params = mobiusParams(1.0, 2.0, 3.0, 2.0, 0.2);
    strip.configure(params, dummyMaterial());
    AABB box = strip.boundingBox();
    double expected_radius = 2.2;
    cr_assert_float_eq(box.min.x, 1.0 - expected_radius, 1e-10);
    cr_assert_float_eq(box.min.y, 2.0 - expected_radius, 1e-10);
    cr_assert_float_eq(box.min.z, 3.0 - 0.2, 1e-10);
    cr_assert_float_eq(box.max.x, 1.0 + expected_radius, 1e-10);
    cr_assert_float_eq(box.max.y, 2.0 + expected_radius, 1e-10);
    cr_assert_float_eq(box.max.z, 3.0 + 0.2, 1e-10);
}

Test(mobius, ray_misses_outside_bounding_box) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{5.0, 5.0, 5.0}, {0.0, 0.0, -1.0}};
    auto hit = strip.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(mobius, ray_hits_mobius_strip) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
}

Test(mobius, hit_point_matches_ray_at_t) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-6);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-6);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-6);
}

Test(mobius, normal_is_unit_length) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-6);
}

Test(mobius, front_face_true_on_outside_hit) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(mobius, uv_coordinates_in_valid_range) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(mobius, tangent_and_bitangent_are_orthogonal_to_normal) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{2.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
    double dot_tangent = dot(hit->tangent, hit->normal);
    double dot_bitangent = dot(hit->bitangent, hit->normal);
    cr_assert_float_eq(dot_tangent, 0.0, 1e-6);
    cr_assert_float_eq(dot_bitangent, 0.0, 1e-6);
    double dot_t_b = dot(hit->tangent, hit->bitangent);
    cr_assert_float_eq(dot_t_b, 0.0, 1e-6);
}

Test(mobius, ray_hits_translated_mobius) {
    MobiusStrip strip;
    auto params = mobiusParams(1.0, 1.0, 1.0, 1.0, 0.1);
    strip.configure(params, dummyMaterial());
    Ray ray{{3.0, 1.0, 1.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
}

Test(mobius, ray_hits_scaled_mobius) {
    MobiusStrip strip;
    auto params = mobiusParams(0.0, 0.0, 0.0, 2.0, 0.2);
    strip.configure(params, dummyMaterial());
    Ray ray{{4.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};
    auto hit = strip.intersect(ray);
    cr_assert(hit.has_value());
}

