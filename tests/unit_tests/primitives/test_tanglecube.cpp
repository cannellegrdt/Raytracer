/*
 * Project: Raytracer
 * File name: test_tanglecube.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Tanglecube plugin - configure validation and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>

#define create tanglecube_create_fn
#define destroy tanglecube_destroy_fn
#include "../../src/plugins/tanglecube.cpp"
#undef create
#undef destroy

static std::unordered_map<std::string, double> tanglecubeParams(
    double x, double y, double z, double s = 1.0)
{
    return {{"x", x}, {"y", y}, {"z", z}, {"s", s}};
}

Test(tanglecube, configure_valid_params_no_throw) {
    Tanglecube tc;
    auto params = tanglecubeParams(0, 0, 0, 1.0);
    cr_assert_no_throw(tc.configure(params, nullptr));
}

Test(tanglecube, configure_default_scale_no_throw) {
    Tanglecube tc;
    auto params = tanglecubeParams(0, 0, 0);
    cr_assert_no_throw(tc.configure(params, nullptr));
}

Test(tanglecube, configure_zero_scale_throws) {
    Tanglecube tc;
    auto params = tanglecubeParams(0, 0, 0, 0.0);
    cr_assert_throw(tc.configure(params, nullptr), std::invalid_argument);
}

Test(tanglecube, configure_negative_scale_throws) {
    Tanglecube tc;
    auto params = tanglecubeParams(0, 0, 0, -1.0);
    cr_assert_throw(tc.configure(params, nullptr), std::invalid_argument);
}

Test(tanglecube, ray_hits_tanglecube_returns_hit) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
}

Test(tanglecube, ray_misses_tanglecube_returns_nullopt) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{10, 10, 10}, {1, 0, 0}};
    auto hit = tc.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(tanglecube, ray_pointing_away_no_hit) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(5, 5, 5, 1.0), nullptr);
    Ray ray{{5, 5, 5}, {1, 0, 0}};
    auto hit = tc.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(tanglecube, front_face_true_when_hitting_outside) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(tanglecube, hit_point_matches_ray_at_t) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-6);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-6);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-6);
}

Test(tanglecube, normal_is_unit_length) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-6);
}

Test(tanglecube, hit_has_valid_uv_coordinates) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 1.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(tanglecube, offset_center_hit) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(3, 3, 3, 1.0), nullptr);
    Ray ray{{3, 3, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
}

Test(tanglecube, scaled_tanglecube_hit) {
    Tanglecube tc;
    tc.configure(tanglecubeParams(0, 0, 0, 2.0), nullptr);
    Ray ray{{0, 0, 0}, {1, 1, 1}};
    auto hit = tc.intersect(ray);
    cr_assert(hit.has_value());
}

Test(tanglecube, plugin_create_returns_non_null) {
    IPrimitive *p = tanglecube_create_fn();
    cr_assert_not_null(p);
    tanglecube_destroy_fn(p);
}

Test(tanglecube, plugin_destroy_deletes_object) {
    IPrimitive *p = tanglecube_create_fn();
    cr_assert_no_throw(tanglecube_destroy_fn(p));
}
