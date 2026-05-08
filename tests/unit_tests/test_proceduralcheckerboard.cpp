/*
 * Project: Raytracer
 * File name: test_proceduralcheckerboard.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for ProceduralCheckerboard material.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <optional>

#include "../../include/materials/ProceduralCheckerboard.hpp"
#include "../../include/Vec3.hpp"
#include "../../include/Ray.hpp"
#include "../../include/HitRecord.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) {
    return std::abs(a - b) < EPS;
}

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

static HitRecord makeHit(Vec3 point, Vec3 normal) {
    return HitRecord{1.0, point, normalize(normal), nullptr, true};
}

Test(proceduralcheckerboard, returns_colorA_at_origin_with_scale_one) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    HitRecord hit = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, colorA));
}

Test(proceduralcheckerboard, returns_colorB_at_offset_position) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    HitRecord hit = makeHit({1.5, 0.5, 0.5}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, colorB));
}

Test(proceduralcheckerboard, returns_no_scattered_ray) {
    Color colorA(1.0, 1.0, 1.0);
    Color colorB(0.0, 0.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    HitRecord hit = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert_not(result.scatteredRay.has_value());
}

Test(proceduralcheckerboard, alternates_on_x_axis) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit1 = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, colorA));

    HitRecord hit2 = makeHit({1.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result2 = mat.scatter(ray, hit2);
    cr_assert(vec3_near(result2.attenuation, colorB));
}

Test(proceduralcheckerboard, alternates_on_y_axis) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit1 = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, colorA));

    HitRecord hit2 = makeHit({0.5, 1.5, 0.5}, {0, 1, 0});
    ScatterResult result2 = mat.scatter(ray, hit2);
    cr_assert(vec3_near(result2.attenuation, colorB));
}

Test(proceduralcheckerboard, alternates_on_z_axis) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit1 = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, colorA));

    HitRecord hit2 = makeHit({0.5, 0.5, 1.5}, {0, 1, 0});
    ScatterResult result2 = mat.scatter(ray, hit2);
    cr_assert(vec3_near(result2.attenuation, colorB));
}

Test(proceduralcheckerboard, xor_pattern_returns_colorA_when_all_even) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result = mat.scatter(ray, hit);
    cr_assert(vec3_near(result.attenuation, colorA));
}

Test(proceduralcheckerboard, xor_pattern_returns_colorB_when_all_odd) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit = makeHit({1.5, 1.5, 1.5}, {0, 1, 0});
    ScatterResult result = mat.scatter(ray, hit);
    cr_assert(vec3_near(result.attenuation, colorB));
}

Test(proceduralcheckerboard, xor_pattern_returns_colorB_when_one_odd) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit = makeHit({1.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result = mat.scatter(ray, hit);
    cr_assert(vec3_near(result.attenuation, colorB));
}

Test(proceduralcheckerboard, respects_scale_factor) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    double scale = 2.0;
    ProceduralCheckerboard mat(colorA, colorB, scale);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit1 = makeHit({1.0, 1.0, 1.0}, {0, 1, 0});
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, colorA));

    HitRecord hit2 = makeHit({3.0, 1.0, 1.0}, {0, 1, 0});
    ScatterResult result2 = mat.scatter(ray, hit2);
    cr_assert(vec3_near(result2.attenuation, colorB));
}

Test(proceduralcheckerboard, negative_positions_work_correctly) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    Ray ray{{0, 0, 5}, {0, 0, -1}};

    HitRecord hit1 = makeHit({-0.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, colorB));

    HitRecord hit2 = makeHit({-1.5, 0.5, 0.5}, {0, 1, 0});
    ScatterResult result2 = mat.scatter(ray, hit2);
    cr_assert(vec3_near(result2.attenuation, colorA));
}

Test(proceduralcheckerboard, ignores_ray_direction) {
    Color colorA(1.0, 0.0, 0.0);
    Color colorB(0.0, 1.0, 0.0);
    ProceduralCheckerboard mat(colorA, colorB, 1.0);

    HitRecord hit = makeHit({0.5, 0.5, 0.5}, {0, 1, 0});

    Ray ray1{{0, 0, 5}, {0, 0, -1}};
    ScatterResult result1 = mat.scatter(ray1, hit);

    Ray ray2{{100, 200, 300}, {1, 1, 1}};
    ScatterResult result2 = mat.scatter(ray2, hit);

    cr_assert(vec3_near(result1.attenuation, result2.attenuation));
}
