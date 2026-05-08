/*
 * Project: Raytracer
 * File name: test_cube.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Cube plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>

#define create cube_create_fn
#define destroy cube_destroy_fn
#include "../../src/plugins/cube.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> cubeParams(
    double x, double y, double z, double s)
{
    return {{"x", x}, {"y", y}, {"z", z}, {"s", s}};
}

Test(cube, configure_valid_params_no_throw) {
    Cube c;
    auto params = cubeParams(0, 0, 0, 1.0);
    cr_assert_no_throw(c.configure(params, nullptr));
}

Test(cube, configure_zero_size_throws) {
    Cube c;
    auto params = cubeParams(0, 0, 0, 0.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cube, configure_negative_size_throws) {
    Cube c;
    auto params = cubeParams(0, 0, 0, -1.0);
    cr_assert_throw(c.configure(params, nullptr), std::invalid_argument);
}

Test(cube, ray_hits_cube_returns_hit) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
}

Test(cube, ray_hits_cube_correct_t) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(cube, ray_misses_cube_returns_nullopt) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{10, 10, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cube, ray_pointing_away_from_cube_no_hit) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, -5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(cube, front_face_true_when_hitting_outside) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(cube, hit_point_lies_on_cube_surface) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.x, 0.0, 1e-10);
    cr_assert_float_eq(hit->point.y, 0.0, 1e-10);
    cr_assert_float_eq(hit->point.z, 1.0, 1e-10);
}

Test(cube, hit_point_matches_ray_at_t) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(cube, normal_is_unit_length) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(cube, normal_points_toward_ray_origin_on_front_hit) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->normal.z > 0.0);
}

Test(cube, hit_has_valid_uv_coordinates) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(cube, offset_center_hit) {
    Cube c;
    c.configure(cubeParams(3, 0, 0, 2.0), nullptr);
    Ray ray{{3, 0, 5}, {0, 0, -1}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 4.0, 1e-10);
}

Test(cube, ray_hits_cube_side) {
    Cube c;
    c.configure(cubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};
    auto hit = c.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.x, 1.0, 1e-10);
}

Test(cube, plugin_create_returns_non_null) {
    IPrimitive *p = cube_create_fn();
    cr_assert_not_null(p);
    cube_destroy_fn(p);
}

Test(cube, plugin_destroy_deletes_object) {
    IPrimitive *p = cube_create_fn();
    cr_assert_no_throw(cube_destroy_fn(p));
}
