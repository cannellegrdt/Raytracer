/*
 * Project: Raytracer
 * File name: test_materials.cpp
 * Description: Criterion unit tests for FlatColor, Reflection, and Transparency materials.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <optional>

#include "../../include/materials/FlatColor.hpp"
#include "../../include/materials/Reflection.hpp"
#include "../../include/materials/Transparency.hpp"
#include "../../include/Vec3.hpp"
#include "../../include/Ray.hpp"
#include "../../include/HitRecord.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) { return std::abs(a - b) < EPS; }

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

static HitRecord makeHit(Vec3 point, Vec3 normal) {
    return HitRecord{1.0, point, normalize(normal), nullptr, true};
}

Test(flatcolor, returns_constant_attenuation) {
    Color col{0.2, 0.4, 0.6};
    FlatColor mat(col);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, col));
}

Test(flatcolor, returns_no_scattered_ray) {
    FlatColor mat(Color{1, 1, 1});
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert_not(result.scatteredRay.has_value());
}

Test(flatcolor, ignores_ray_and_hit_parameters) {
    FlatColor mat(Color{0.5, 0.5, 0.5});
    
    HitRecord hit = makeHit({10, 20, 30}, {1, 0, 0});
    Ray ray{{100, 200, 300}, {1, 1, 1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, Color{0.5, 0.5, 0.5}));
    cr_assert_not(result.scatteredRay.has_value());
}

Test(reflection, returns_reflected_ray) {
    Color col{0.8, 0.8, 0.8};
    Reflection mat(col);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
}

Test(reflection, reflected_direction_is_correct) {
    Reflection mat(Color{1, 1, 1});
    
    Vec3 normal{0, 1, 0};
    HitRecord hit = makeHit({0, 0, 0}, normal);
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    Vec3 expectedDir = normalize(ray.direction - 2 * dot(ray.direction, normal) * normal);
    cr_assert(vec3_near(result.scatteredRay->direction, expectedDir));
}

Test(reflection, reflected_ray_origin_is_offset_from_hit_point) {
    Reflection mat(Color{1, 1, 1});
    
    HitRecord hit = makeHit({5, 0, -3}, {0, 1, 0});
    Ray ray{{5, 1, -2}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
    static constexpr double OFFSET_EPS = 1e-4;
    cr_assert(near(result.scatteredRay->origin.x, hit.point.x));
    cr_assert(near(result.scatteredRay->origin.y, hit.point.y + OFFSET_EPS));
    cr_assert(near(result.scatteredRay->origin.z, hit.point.z));
}

Test(reflection, returns_correct_attenuation) {
    Color col{0.3, 0.5, 0.7};
    Reflection mat(col);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, col));
}

Test(reflection, reflected_ray_is_unit_vector) {
    Reflection mat(Color{1, 1, 1});
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 2, 2}, {1, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
    cr_assert(near(length(result.scatteredRay->direction), 1.0));
}

Test(transparency, returns_transmitted_ray) {
    Color col{0.9, 0.9, 0.9};
    Transparency mat(col);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
}

Test(transparency, transmitted_ray_keeps_original_direction) {
    Transparency mat(Color{1, 1, 1});
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 1}, {0.5, -0.5, -0.707}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
    cr_assert(vec3_near(result.scatteredRay->direction, ray.direction));
}

Test(transparency, transmitted_ray_origin_is_biased_inside_surface) {
    Transparency mat(Color{1, 1, 1});

    HitRecord hit = makeHit({3, -2, 5}, {0, 1, 0});
    Ray ray{{3, 0, 6}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.scatteredRay.has_value());
    Vec3 expected = hit.point - 1e-4 * hit.normal;
    cr_assert(vec3_near(result.scatteredRay->origin, expected));
}

Test(transparency, returns_correct_attenuation) {
    Color col{0.4, 0.6, 0.8};
    Transparency mat(col);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 1, 1}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, col));
}

Test(transparency, ray_passes_through_regardless_of_normal) {
    Transparency mat(Color{1, 1, 1});
    
    Vec3 differentNormal{1, 0, 0};
    HitRecord hit = makeHit({0, 0, 0}, differentNormal);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(result.scatteredRay.has_value());
    cr_assert(vec3_near(result.scatteredRay->direction, ray.direction));
}