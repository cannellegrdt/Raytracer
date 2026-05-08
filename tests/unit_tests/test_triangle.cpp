/*
 * Project: Raytracer
 * File name: test_triangle.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Triangle plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>

#define create triangle_create_fn
#define destroy triangle_destroy_fn
#include "../../src/plugins/triangle.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> triangleParams(
    double v0x, double v0y, double v0z,
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z)
{
    return {
        {"v0x", v0x}, {"v0y", v0y}, {"v0z", v0z},
        {"v1x", v1x}, {"v1y", v1y}, {"v1z", v1z},
        {"v2x", v2x}, {"v2y", v2y}, {"v2z", v2z}
    };
}

Test(triangle, configure_valid_params_no_throw) {
    Triangle t;
    auto params = triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0);
    cr_assert_no_throw(t.configure(params, nullptr));
}

Test(triangle, configure_missing_param_throws) {
    Triangle t;
    std::unordered_map<std::string, double> params = {{"v0x", 0}};
    cr_assert_throw(t.configure(params, nullptr), std::out_of_range);
}

Test(triangle, ray_hits_triangle) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.1, 0.1, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
}

Test(triangle, ray_misses_triangle_outside_v0v1) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{-1, -1, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, ray_misses_triangle_outside_v1v2) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{1, 1, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, ray_misses_triangle_outside_v2v0) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{-0.5, 1, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, ray_parallel_to_triangle_no_hit) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 1}, {1, 0, 0}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, ray_pointing_away_from_triangle_no_hit) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.1, 0.1, 1}, {0, 0, 1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, ray_behind_triangle_no_hit) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.1, 0.1, -1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(triangle, correct_t_value) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 1.0, 1e-10);
}

Test(triangle, hit_point_lies_on_triangle) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.x, 0.25, 1e-10);
    cr_assert_float_eq(hit->point.y, 0.25, 1e-10);
    cr_assert_float_eq(hit->point.z, 0.0, 1e-10);
}

Test(triangle, hit_point_matches_ray_at_t) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(triangle, normal_is_correct_direction) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->normal.x, 0.0, 1e-10);
    cr_assert_float_eq(hit->normal.y, 0.0, 1e-10);
    cr_assert_float_eq(hit->normal.z, 1.0, 1e-10);
}

Test(triangle, normal_is_unit_length) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(triangle, front_face_when_ray_comes_from_front) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(triangle, back_face_when_ray_comes_from_behind) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, -1}, {0, 0, 1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_not(hit->frontFace);
}

Test(triangle, uv_coordinates_in_valid_range) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(triangle, uv_at_v0) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 0, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 0.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 0.0, 1e-10);
}

Test(triangle, uv_at_v1) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{1, 0, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 1.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 0.0, 1e-10);
}

Test(triangle, uv_at_v2) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 0.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 1.0, 1e-10);
}

Test(triangle, tangent_is_edge1_direction) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 1, 0), nullptr);
    Ray ray{{0.25, 0.25, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->tangent.x, 1.0, 1e-10);
    cr_assert_float_eq(hit->tangent.y, 0.0, 1e-10);
    cr_assert_float_eq(hit->tangent.z, 0.0, 1e-10);
}

Test(triangle, plugin_create_returns_non_null) {
    IPrimitive *p = triangle_create_fn();
    cr_assert_not_null(p);
    triangle_destroy_fn(p);
}

Test(triangle, plugin_destroy_deletes_object) {
    IPrimitive *p = triangle_create_fn();
    cr_assert_no_throw(triangle_destroy_fn(p));
}

Test(triangle, ray_hits_3d_triangle) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 1, 0, 0, 0, 0, 1), nullptr);
    Ray ray{{0.1, -0.1, 0.1}, {0, 1, 0}};
    auto hit = t.intersect(ray);
    cr_assert(hit.has_value());
}

Test(triangle, degenerate_triangle_normal_is_zero) {
    Triangle t;
    t.configure(triangleParams(0, 0, 0, 0, 0, 0, 0, 0, 0), nullptr);
    Ray ray{{0, 0, 1}, {0, 0, -1}};
    auto hit = t.intersect(ray);
    cr_assert_not(hit.has_value());
}
