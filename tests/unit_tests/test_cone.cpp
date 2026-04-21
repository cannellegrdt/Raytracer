/*
 * Project: Raytracer
 * File name: test_cone.cpp
 * Description: Criterion unit tests for the Cone plugin - configure validation and ray intersection.
 *
 * cone.cpp is included directly (not linked as .so) so we can test the class in isolation.
 * The extern "C" symbols are renamed to avoid linker clashes with other plugin test objects.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <unordered_map>
#include <string>

#define create cone_create_fn
#define destroy cone_destroy_fn
#include "../../src/plugins/cone.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> coneParams(
    double x, double y, double z,
    double ax, double ay, double az,
    double angle) {
    return {{"x", x}, {"y", y}, {"z", z}, {"ax", ax}, {"ay", ay}, {"az", az}, {"angle", angle}};
}

Test(cone, configure_valid_params_no_throw) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0);
    cr_assert_no_throw(c.configure(params, nullptr));
}

Test(cone, configure_zero_axis_throws) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 0, 0,  M_PI / 4.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cone, configure_zero_angle_throws) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 1, 0,  0.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cone, configure_angle_at_pi_over_2_throws) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 1, 0,  M_PI / 2.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cone, configure_angle_above_pi_over_2_throws) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 1, 0,  M_PI);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cone, configure_negative_angle_throws) {
    Cone c;
    auto params = coneParams(0, 0, 0,  0, 1, 0,  -0.1);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cone, ray_hits_cone) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
}

Test(cone, ray_hits_cone_correct_t) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 15.0, 1e-10);
}

Test(cone, ray_misses_cone) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{100, 0, 0}, {1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cone, ray_from_apex_along_negative_axis_no_hit) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cone, hit_point_matches_ray_at_t) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(cone, hit_point_lies_on_cone_surface) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    double lhs = hit->point.y * hit->point.y;
    double rhs = hit->point.x * hit->point.x + hit->point.z * hit->point.z;
    cr_assert_float_eq(lhs, rhs, 1e-8);
}

Test(cone, normal_is_unit_length) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(cone, front_face_set_correctly) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 1, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    double facingDot = dot(ray.direction, hit->normal);
    cr_assert(facingDot < 0.0);
    cr_assert(hit->frontFace);
}

Test(cone, non_unit_axis_is_normalized) {
    Cone c;
    c.configure(coneParams(0, 0, 0,  0, 5, 0,  M_PI / 4.0), nullptr);
    Ray ray{{0, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 15.0, 1e-10);
}

Test(cone, plugin_create_returns_non_null) {
    IPrimitive *p = cone_create_fn();
    cr_assert_not_null(p);
    cone_destroy_fn(p);
}

Test(cone, plugin_destroy_deletes_object) {
    IPrimitive *p = cone_create_fn();
    cr_assert_no_throw(cone_destroy_fn(p));
}
