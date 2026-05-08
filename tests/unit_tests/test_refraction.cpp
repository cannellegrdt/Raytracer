/*
 * Project: Raytracer
 * File name: test_refraction.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for Refraction material.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <optional>

#include "Refraction.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) { return std::abs(a - b) < EPS; }

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

static HitRecord makeHit(Vec3 point, Vec3 normal, bool frontFace = true) {
    return HitRecord{1.0, point, normalize(normal), nullptr, frontFace, {0.0, 0.0}, Vec3(0,0,0), Vec3(0,0,0)};
}

Test(refraction, returns_refracted_ray) {
    Color col{0.9, 0.9, 1.0};
    Refraction mat(col, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 0}, {0, -1, 0}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
}

Test(refraction, refracted_ray_direction_changes) {
    Refraction mat(Color{1, 1, 1}, 1.5);

    Vec3 normal{0, 1, 0};
    HitRecord hit = makeHit({0, 0, 0}, normal);
    Ray ray{{0, 2, 0}, {0, -1, 0.5}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
    cr_assert_not(vec3_near(result.scatteredRay->direction, normalize(ray.direction)));
}

Test(refraction, enters_material_front_face) {
    Refraction mat(Color{1, 1, 1}, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, true);
    Ray ray{{0, 2, 0}, {0, -1, 0}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
    Vec3 expectedOrigin = hit.point - 1e-4 * hit.normal;
    cr_assert(vec3_near(result.scatteredRay->origin, expectedOrigin));
}

Test(refraction, exits_material_back_face) {
    Refraction mat(Color{1, 1, 1}, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, false);
    Ray ray{{0, -2, 0}, {0, 1, 0}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
    Vec3 expectedOrigin = hit.point + 1e-4 * hit.normal;
    cr_assert(vec3_near(result.scatteredRay->origin, expectedOrigin));
}

Test(refraction, total_internal_reflection) {
    Refraction mat(Color{1, 1, 1}, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, false);
    Ray ray{{0, -0.1, 0}, {0, 0.1, 1.0}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
}

Test(refraction, returns_correct_attenuation) {
    Color col{0.8, 0.6, 0.4};
    Refraction mat(col, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 2, 0}, {0, -1, 0}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, col));
}

Test(refraction, refracted_ray_is_unit_vector) {
    Refraction mat(Color{1, 1, 1}, 1.5);

    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 2, 1}, {0, -1, 0.5}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
    cr_assert(near(length(result.scatteredRay->direction), 1.0));
}

Test(refraction, different_ior_values) {
    Refraction waterMat(Color{1, 1, 1}, 1.33);
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 2, 0}, {0, -1, 0.3}};

    ScatterResult result = waterMat.scatter(ray, hit);
    cr_assert(result.scatteredRay.has_value());

    Refraction diamondMat(Color{1, 1, 1}, 2.42);
    result = diamondMat.scatter(ray, hit);
    cr_assert(result.scatteredRay.has_value());
}
