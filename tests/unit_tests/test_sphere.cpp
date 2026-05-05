/*
 * Project: Raytracer
 * File name: test_sphere.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Sphere plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>

#define create sphere_create_fn
#define destroy sphere_destroy_fn
#include "../../src/plugins/sphere.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> sphereParams(
    double x, double y, double z, double r)
{
    return {{"x", x}, {"y", y}, {"z", z}, {"r", r}};
}

Test(sphere, configure_valid_params_no_throw) {
    Sphere s;
    auto params = sphereParams(0, 0, 0, 1.0);
    cr_assert_no_throw(s.configure(params, nullptr));
}

Test(sphere, configure_negative_radius_throws) {
    Sphere s;
    auto params = sphereParams(0, 0, 0, -1.0);
    cr_assert_throw(s.configure(params, nullptr), std::invalid_argument);
}

Test(sphere, configure_zero_radius_throws) {
    Sphere s;
    auto params = sphereParams(0, 0, 0, 0.0);
    cr_assert_throw(s.configure(params, nullptr), std::invalid_argument);
}

Test(sphere, ray_hits_sphere_returns_hit) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
}

Test(sphere, ray_hits_sphere_correct_t) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(sphere, ray_misses_sphere_returns_nullopt) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{10, 10, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(sphere, ray_pointing_away_from_sphere_no_hit) {
    Sphere s;
    s.configure(sphereParams(0, 0, 5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(sphere, front_face_true_when_hitting_outside) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(sphere, normal_points_toward_ray_origin_on_front_hit) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->normal.z > 0.0);
}

Test(sphere, ray_inside_sphere_hits_back_face) {
    Sphere s;
    s.configure(sphereParams(0, 0, 0, 10.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_not(hit->frontFace);
}

Test(sphere, hit_point_lies_on_sphere_surface) {
    Sphere s;
    s.configure(sphereParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    double dist = length(hit->point);
    cr_assert_float_eq(dist, 1.0, 1e-10);
}

Test(sphere, hit_point_matches_ray_at_t) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(sphere, normal_is_unit_length) {
    Sphere s;
    s.configure(sphereParams(0, 0, -5, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(sphere, offset_center_hit) {
    Sphere s;
    s.configure(sphereParams(3, 0, -5, 1.0), nullptr);
    Ray ray{{3, 0, 0}, {0, 0, -1}};
    auto hit = s.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(sphere, plugin_create_returns_non_null) {
    IPrimitive *p = sphere_create_fn();
    cr_assert_not_null(p);
    sphere_destroy_fn(p);
}

Test(sphere, plugin_destroy_deletes_object) {
    IPrimitive *p = sphere_create_fn();
    cr_assert_no_throw(sphere_destroy_fn(p));
}
