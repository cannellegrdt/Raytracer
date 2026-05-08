/*
 * Project: Raytracer
 * File name: test_proceduralmarble.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for ProceduralMarble material - scatter function and color computation.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <optional>

#include "../../include/materials/ProceduralMarble.hpp"
#include "../../include/Vec3.hpp"
#include "../../include/Ray.hpp"
#include "../../include/HitRecord.hpp"
#include "../../include/Color.hpp"

static constexpr double EPS = 1e-9;

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return std::abs(a.x - b.x) < EPS && std::abs(a.y - b.y) < EPS && std::abs(a.z - b.z) < EPS;
}

static HitRecord makeHit(Vec3 point, Vec3 normal) {
    return HitRecord{1.0, point, normalize(normal), nullptr, true};
}

Test(proceduralmarble, constructor_creates_valid_instance) {
    Color colorA{0.2, 0.4, 0.6};
    Color colorB{0.8, 0.6, 0.4};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    cr_assert(true);
}

Test(proceduralmarble, scatter_returns_no_scattered_ray) {
    Color colorA{0.2, 0.4, 0.6};
    Color colorB{0.8, 0.6, 0.4};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = marble.scatter(ray, hit);
    
    cr_assert_not(result.scatteredRay.has_value());
}

Test(proceduralmarble, scatter_returns_valid_attenuation) {
    Color colorA{0.2, 0.4, 0.6};
    Color colorB{0.8, 0.6, 0.4};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = marble.scatter(ray, hit);
    
    cr_assert(result.attenuation.x >= 0.0 && result.attenuation.x <= 1.0);
    cr_assert(result.attenuation.y >= 0.0 && result.attenuation.y <= 1.0);
    cr_assert(result.attenuation.z >= 0.0 && result.attenuation.z <= 1.0);
}

Test(proceduralmarble, scatter_attenuation_in_range) {
    Color colorA{0.0, 0.0, 0.0};
    Color colorB{1.0, 1.0, 1.0};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = marble.scatter(ray, hit);
    
    cr_assert(result.attenuation.x >= 0.0 && result.attenuation.x <= 1.0);
    cr_assert(result.attenuation.y >= 0.0 && result.attenuation.y <= 1.0);
    cr_assert(result.attenuation.z >= 0.0 && result.attenuation.z <= 1.0);
}

Test(proceduralmarble, different_points_produce_different_colors) {
    Color colorA{0.0, 0.0, 0.0};
    Color colorB{1.0, 1.0, 1.0};
    ProceduralMarble marble(colorA, colorB, 5.0, 2.0, 4);
    
    HitRecord hit1 = makeHit({0, 0, 0}, {0, 1, 0});
    HitRecord hit2 = makeHit({0, 0, 10}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result1 = marble.scatter(ray, hit1);
    ScatterResult result2 = marble.scatter(ray, hit2);
    
    cr_assert_not(vec3_near(result1.attenuation, result2.attenuation));
}

Test(proceduralmarble, same_point_produces_same_color) {
    Color colorA{0.2, 0.4, 0.6};
    Color colorB{0.8, 0.6, 0.4};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    
    HitRecord hit = makeHit({1.5, 2.5, 3.5}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result1 = marble.scatter(ray, hit);
    ScatterResult result2 = marble.scatter(ray, hit);
    
    cr_assert(vec3_near(result1.attenuation, result2.attenuation));
}

Test(proceduralmarble, ignores_ray_direction) {
    Color colorA{0.5, 0.5, 0.5};
    Color colorB{1.0, 1.0, 1.0};
    ProceduralMarble marble(colorA, colorB, 1.0, 1.0, 4);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray1{{0, 0, 5}, {0, 0, -1}};
    Ray ray2{{0, 0, 5}, {1, 1, -1}};
    
    ScatterResult result1 = marble.scatter(ray1, hit);
    ScatterResult result2 = marble.scatter(ray2, hit);
    
    cr_assert(vec3_near(result1.attenuation, result2.attenuation));
}

Test(proceduralmarble, color_lerp_with_mid_range_noise) {
    Color colorA{0.0, 0.0, 0.0};
    Color colorB{1.0, 1.0, 1.0};
    ProceduralMarble marble(colorA, colorB, 1.0, 0.0, 1);
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = marble.scatter(ray, hit);
    
    cr_assert(result.attenuation.x >= 0.0 && result.attenuation.x <= 1.0);
    cr_assert(result.attenuation.y >= 0.0 && result.attenuation.y <= 1.0);
    cr_assert(result.attenuation.z >= 0.0 && result.attenuation.z <= 1.0);
}

Test(proceduralmarble, handles_different_octave_values) {
    Color colorA{0.0, 0.0, 0.0};
    Color colorB{1.0, 1.0, 1.0};
    ProceduralMarble marble1(colorA, colorB, 2.0, 5.0, 2);
    ProceduralMarble marble2(colorA, colorB, 2.0, 5.0, 8);
    
    HitRecord hit = makeHit({1.0, 2.0, 3.0}, {0, 1, 0});
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result1 = marble1.scatter(ray, hit);
    ScatterResult result2 = marble2.scatter(ray, hit);
    
    cr_assert(result1.attenuation.x >= 0.0 && result1.attenuation.x <= 1.0);
    cr_assert(result2.attenuation.x >= 0.0 && result2.attenuation.x <= 1.0);
}
