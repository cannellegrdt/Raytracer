/*
 * Project: Raytracer
 * File name: test_limited_cone.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the LimitedCone plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <unordered_map>
#include <string>

#define create limited_cone_create_fn
#define destroy limited_cone_destroy_fn
#include "../../src/plugins/limited_cone.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> lconeParams(
    double x, double y, double z,
    double ax, double ay, double az,
    double angle, double h) {
    return {{"x", x}, {"y", y}, {"z", z}, {"ax", ax}, {"ay", ay}, {"az", az},
            {"angle", angle}, {"h", h}};
}

Test(limited_cone, configure_valid_params_no_throw) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  5.0);
    cr_assert_no_throw(c.configure(params, nullptr));
}

Test(limited_cone, configure_zero_axis_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 0, 0,  M_PI / 4.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, configure_zero_angle_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  0.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, configure_angle_at_pi_over_2_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 2.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, configure_negative_angle_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  -0.1,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, configure_zero_height_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  0.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, configure_negative_height_throws) {
    LimitedCone c;
    auto params = lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  -2.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cone, ray_hits_body) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
}

Test(limited_cone, ray_hits_body_correct_t) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 15.0, 1e-10);
}

Test(limited_cone, ray_misses_above_height) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  3.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(limited_cone, ray_hits_cap) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  5.0), nullptr);
    Ray ray{{0, 10, 0}, {0, -1, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, 5.0, 1e-10);
}

Test(limited_cone, ray_misses_entirely) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  5.0), nullptr);
    Ray ray{{100, 0, 0}, {1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(limited_cone, hit_point_matches_ray_at_t) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(limited_cone, hit_point_within_height) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->point.y >= 0.0 && hit->point.y <= 10.0);
}

Test(limited_cone, normal_is_unit_length) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(limited_cone, front_face_set_correctly) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(dot(ray.direction, hit->normal) < 0.0);
    cr_assert(hit->frontFace);
}

Test(limited_cone, non_unit_axis_is_normalized) {
    LimitedCone c;
    c.configure(lconeParams(0, 0, 0,  0, 4, 0,  M_PI / 4.0,  10.0), nullptr);
    Ray ray{{0, 5, 10}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 15.0, 1e-10);
}

Test(limited_cone, plugin_create_returns_non_null) {
    IPrimitive *p = limited_cone_create_fn();
    cr_assert_not_null(p);
    limited_cone_destroy_fn(p);
}

Test(limited_cone, plugin_destroy_deletes_object) {
    IPrimitive *p = limited_cone_create_fn();
    cr_assert_no_throw(limited_cone_destroy_fn(p));
}
