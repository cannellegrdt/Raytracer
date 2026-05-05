/*
 * Project: Raytracer
 * File name: test_limited_cylinder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the LimitedCylinder plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <unordered_map>
#include <string>

#define create limited_cylinder_create_fn
#define destroy limited_cylinder_destroy_fn
#include "../../src/plugins/limited_cylinder.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> lcylParams(
    double x, double y, double z,
    double ax, double ay, double az,
    double r, double h) {
    return {{"x", x}, {"y", y}, {"z", z}, {"ax", ax}, {"ay", ay}, {"az", az},
            {"r", r}, {"h", h}};
}

Test(limited_cylinder, configure_valid_params_no_throw) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0);
    cr_assert_no_throw(c.configure(params, nullptr));
}

Test(limited_cylinder, configure_zero_axis_throws) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 0, 0,  1.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cylinder, configure_zero_radius_throws) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 1, 0,  0.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cylinder, configure_negative_radius_throws) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 1, 0,  -1.0,  5.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cylinder, configure_zero_height_throws) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 1, 0,  1.0,  0.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cylinder, configure_negative_height_throws) {
    LimitedCylinder c;
    auto params = lcylParams(0, 0, 0,  0, 1, 0,  1.0,  -3.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(limited_cylinder, ray_hits_body) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
}

Test(limited_cylinder, ray_hits_body_correct_t) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(limited_cylinder, ray_misses_above_height) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  3.0), nullptr);
    Ray ray{{5, 10, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(limited_cylinder, ray_misses_below_base) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, -2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(limited_cylinder, ray_hits_bottom_cap) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{0, -5, 0}, {0.1, 1, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, 0.0, 1e-10);
}

Test(limited_cylinder, ray_hits_top_cap) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{0, 8, 0}, {0.1, -1, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, 5.0, 1e-10);
}

Test(limited_cylinder, ray_misses_entirely) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{100, 0, 0}, {1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(limited_cylinder, hit_point_matches_ray_at_t) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(limited_cylinder, hit_point_within_height) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->point.y >= 0.0 && hit->point.y <= 5.0);
}

Test(limited_cylinder, normal_is_unit_length) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(limited_cylinder, body_normal_is_radial) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->normal.y, 0.0, 1e-10);
}

Test(limited_cylinder, front_face_set_correctly) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(dot(ray.direction, hit->normal) < 0.0);
    cr_assert(hit->frontFace);
}

Test(limited_cylinder, non_unit_axis_is_normalized) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 3, 0,  1.0,  5.0), nullptr);
    Ray ray{{5, 2, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(limited_cylinder, closest_hit_wins_between_body_and_cap) {
    LimitedCylinder c;
    c.configure(lcylParams(0, 0, 0,  0, 1, 0,  1.0,  5.0), nullptr);
    Ray ray{{0, -5, 0}, {0.1, 1, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, 0.0, 1e-10);
}

Test(limited_cylinder, plugin_create_returns_non_null) {
    IPrimitive *p = limited_cylinder_create_fn();
    cr_assert_not_null(p);
    limited_cylinder_destroy_fn(p);
}

Test(limited_cylinder, plugin_destroy_deletes_object) {
    IPrimitive *p = limited_cylinder_create_fn();
    cr_assert_no_throw(limited_cylinder_destroy_fn(p));
}
