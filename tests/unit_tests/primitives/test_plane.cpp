/*
 * Project: Raytracer
 * File name: test_plane.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Plane plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>

#define create plane_create_fn
#define destroy plane_destroy_fn
#include "../../src/plugins/plane.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> planeParams(
    double x, double y, double z,
    double nx, double ny, double nz)
{
    return {{"x", x}, {"y", y}, {"z", z}, {"nx", nx}, {"ny", ny}, {"nz", nz}};
}

Test(plane, configure_valid_params_no_throw) {
    Plane p;
    auto params = planeParams(0, 0, 0, 0, 1, 0);
    cr_assert_no_throw(p.configure(params, nullptr));
}

Test(plane, configure_zero_normal_throws) {
    Plane p;
    auto params = planeParams(0, 0, 0, 0, 0, 0);
    cr_assert_throw(p.configure(params, nullptr), std::invalid_argument);
}

Test(plane, normal_is_normalized_after_configure) {
    Plane p;
    p.configure(planeParams(0, 0, 0, 0, 5, 0), nullptr);
    Ray ray{{0, 5, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(plane, ray_hits_horizontal_plane) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
}

Test(plane, ray_hits_plane_correct_t) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 3.0, 1e-10);
}

Test(plane, ray_parallel_to_plane_no_hit) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {1, 0, 0}};
    auto hit = p.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(plane, ray_pointing_away_from_plane_no_hit) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, 1, 0}};
    auto hit = p.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(plane, hit_point_lies_on_plane) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, -3.0, 1e-10);
}

Test(plane, hit_point_matches_ray_at_t) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{1, 0, 2}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(plane, front_face_when_ray_comes_from_normal_side) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
    cr_assert(hit->normal.y > 0.0);
}

Test(plane, back_face_when_ray_comes_from_opposite_side) {
    Plane p;
    p.configure(planeParams(0, 3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, 1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_not(hit->frontFace);
    cr_assert(hit->normal.y < 0.0);
}

Test(plane, normal_is_unit_length_on_hit) {
    Plane p;
    p.configure(planeParams(0, -3, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {0, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(plane, diagonal_ray_hits_plane) {
    Plane p;
    p.configure(planeParams(0, -1, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 0}, {1, -1, 0}};
    auto hit = p.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.y, -1.0, 1e-10);
}

Test(plane, plugin_create_returns_non_null) {
    IPrimitive *p = plane_create_fn();
    cr_assert_not_null(p);
    plane_destroy_fn(p);
}

Test(plane, plugin_destroy_deletes_object) {
    IPrimitive *p = plane_create_fn();
    cr_assert_no_throw(plane_destroy_fn(p));
}
