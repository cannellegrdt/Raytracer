/*
 * Project: Raytracer
 * File name: test_cylinder.cpp
 * Description: Criterion unit tests for the Cylinder plugin - configure validation and ray intersection.
 *
 * cylinder.cpp is included directly (not linked as .so) so we can test the class in isolation.
 * The extern "C" symbols are renamed to avoid linker clashes with other plugin test objects.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <unordered_map>
#include <string>

#define create cylinder_create_fn
#define destroy cylinder_destroy_fn
#include "../../src/plugins/cylinder.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> cylParams(
    double x, double y, double z,
    double ax, double ay, double az,
    double r) {
    return {{"x", x}, {"y", y}, {"z", z}, {"ax", ax}, {"ay", ay}, {"az", az}, {"r", r}};
}

Test(cylinder, configure_valid_params_no_throw) {
    Cylinder c;
    auto params = cylParams(0, 0, 0,  0, 1, 0,  1.0);
    cr_assert_no_throw(c.configure(params, nullptr));
}

Test(cylinder, configure_zero_axis_throws) {
    Cylinder c;
    auto params = cylParams(0, 0, 0,  0, 0, 0,  1.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cylinder, configure_zero_radius_throws) {
    Cylinder c;
    auto params = cylParams(0, 0, 0,  0, 1, 0,  0.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cylinder, configure_negative_radius_throws) {
    Cylinder c;
    auto params = cylParams(0, 0, 0,  0, 1, 0,  -1.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cylinder, ray_hits_cylinder) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
}

Test(cylinder, ray_hits_cylinder_correct_t) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(cylinder, ray_misses_cylinder) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cylinder, ray_moving_away_misses) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, 1}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cylinder, hit_point_matches_ray_at_t) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(cylinder, hit_point_at_correct_radius) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  2.0), nullptr);
    Ray ray{{5, 3, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    double radial = std::sqrt(hit->point.x * hit->point.x + hit->point.z * hit->point.z);
    cr_assert_float_eq(radial, 2.0, 1e-10);
}

Test(cylinder, normal_is_unit_length) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(cylinder, normal_is_radial) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->normal.y, 0.0, 1e-10);
}

Test(cylinder, front_face_set_correctly) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(dot(ray.direction, hit->normal) < 0.0);
    cr_assert(hit->frontFace);
}

Test(cylinder, non_unit_axis_is_normalized) {
    Cylinder c;
    c.configure(cylParams(0, 0, 0,  0, 3, 0,  1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(cylinder, offset_center_hit) {
    Cylinder c;
    c.configure(cylParams(3, 0, 0,  0, 1, 0,  1.0), nullptr);
    Ray ray{{8, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(cylinder, plugin_create_returns_non_null) {
    IPrimitive *p = cylinder_create_fn();
    cr_assert_not_null(p);
    cylinder_destroy_fn(p);
}

Test(cylinder, plugin_destroy_deletes_object) {
    IPrimitive *p = cylinder_create_fn();
    cr_assert_no_throw(cylinder_destroy_fn(p));
}
