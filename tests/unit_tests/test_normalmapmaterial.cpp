/*
 * Project: Raytracer
 * File name: test_normalmapmaterial.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for NormalMapMaterial.
 */

#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <memory>

#include "NormalMapMaterial.hpp"
#include "FlatColor.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"

static constexpr double EPS = 1e-9;
static constexpr double LENGTH_EPS = 1e-4;

static bool near(double a, double b) { return std::abs(a - b) < EPS; }

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

static HitRecord makeHit(Vec3 point, Vec3 normal, double u, double v, Vec3 tangent, Vec3 bitangent) {
    return HitRecord{1.0, point, normalize(normal), nullptr, true, {u, v}, tangent, bitangent};
}

struct StubMaterial : IMaterial {
    ScatterResult result;
    mutable Ray lastRay;
    mutable HitRecord lastHit;
    std::optional<SpecularParams> specularParams = std::nullopt;

    explicit StubMaterial(const Color &col) {
        result.attenuation = col;
        result.scatteredRay = std::nullopt;
    }

    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override {
        lastRay = ray;
        lastHit = hit;
        return result;
    }

    std::optional<SpecularParams> getSpecular() const override { return specularParams; }
};

Test(normalmapmaterial, load_valid_ppm_no_throw) {
    cr_assert_no_throw(NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "textures/checkerboard.ppm"));
}

Test(normalmapmaterial, load_invalid_file_does_not_throw) {
    cr_assert_no_throw(NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "nonexistent.ppm"));
}

Test(normalmapmaterial, empty_normal_map_returns_base_result_unchanged) {
    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{0.5, 0.5, 0.5}), "nonexistent.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.5, 0.5, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, Color{0.5, 0.5, 0.5}));
    cr_assert_not(result.modifiedNormal.has_value());
}

Test(normalmapmaterial, valid_normal_map_modifies_normal) {
    std::ofstream file("textures/test_normal_2x2.ppm");
    file << "P3\n2 2\n255\n";
    file << "128 128 255 ";
    file << "128 128 255\n";
    file << "128 128 255 ";
    file << "128 128 255\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "textures/test_normal_2x2.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.25, 0.25, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.modifiedNormal.has_value());
    cr_assert(near(length(result.modifiedNormal.value()), 1.0));

    std::filesystem::remove("textures/test_normal_2x2.ppm");
}

Test(normalmapmaterial, normal_map_with_zero_tangent_returns_base_result) {
    std::ofstream file("textures/test_normal_zero.ppm");
    file << "P3\n1 1\n255\n";
    file << "128 128 255\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{0.5, 0.5, 0.5}), "textures/test_normal_zero.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{0, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.5, 0.5, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, Color{0.5, 0.5, 0.5}));
    cr_assert_not(result.modifiedNormal.has_value());

    std::filesystem::remove("textures/test_normal_zero.ppm");
}

Test(normalmapmaterial, normal_map_with_zero_bitangent_returns_base_result) {
    std::ofstream file("textures/test_normal_zerob.ppm");
    file << "P3\n1 1\n255\n";
    file << "128 128 255\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{0.5, 0.5, 0.5}), "textures/test_normal_zerob.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 0};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.5, 0.5, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(vec3_near(result.attenuation, Color{0.5, 0.5, 0.5}));
    cr_assert_not(result.modifiedNormal.has_value());

    std::filesystem::remove("textures/test_normal_zerob.ppm");
}

Test(normalmapmaterial, uv_out_of_range_wraps) {
    std::ofstream file("textures/test_normal_wrap.ppm");
    file << "P3\n2 2\n255\n";
    file << "255 0 0 ";
    file << "0 255 0\n";
    file << "0 0 255 ";
    file << "255 255 255\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "textures/test_normal_wrap.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 1.5, 0.25, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.modifiedNormal.has_value());

    std::filesystem::remove("textures/test_normal_wrap.ppm");
}

Test(normalmapmaterial, get_specular_delegates_to_base_material) {
    auto base = std::make_shared<StubMaterial>(Color{1,1,1});
    base->specularParams = SpecularParams{Color{1.0, 1.0, 1.0}, 32};

    NormalMapMaterial mat(base, "nonexistent.ppm");

    auto spec = mat.getSpecular();
    cr_assert(spec.has_value());
    cr_assert_eq(spec->shininess, 32);
    cr_assert(vec3_near(spec->ks, Color{1.0, 1.0, 1.0}));
}

Test(normalmapmaterial, get_specular_returns_nullopt_when_base_returns_nullopt) {
    auto base = std::make_shared<StubMaterial>(Color{1,1,1});
    base->specularParams = std::nullopt;

    NormalMapMaterial mat(base, "nonexistent.ppm");

    auto spec = mat.getSpecular();
    cr_assert_not(spec.has_value());
}

Test(normalmapmaterial, modified_normal_has_unit_length) {
    std::ofstream file("textures/test_normal_unit.ppm");
    file << "P3\n1 1\n255\n";
    file << "255 128 0\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "textures/test_normal_unit.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.5, 0.5, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.modifiedNormal.has_value());
    cr_assert(near(length(result.modifiedNormal.value()), 1.0));

    std::filesystem::remove("textures/test_normal_unit.ppm");
}

Test(normalmapmaterial, normal_map_full_blue_gives_identity_normal) {
    std::ofstream file("textures/test_normal_blue.ppm");
    file << "P3\n1 1\n255\n";
    file << "128 128 255\n";
    file.close();

    NormalMapMaterial mat(std::make_shared<FlatColor>(Color{1,1,1}), "textures/test_normal_blue.ppm");

    Vec3 normal{0, 1, 0};
    Vec3 tangent{1, 0, 0};
    Vec3 bitangent{0, 0, 1};
    HitRecord hit = makeHit({0, 0, 0}, normal, 0.5, 0.5, tangent, bitangent);
    Ray ray{{0, 0, 5}, {0, 0, -1}};

    ScatterResult result = mat.scatter(ray, hit);

    cr_assert(result.modifiedNormal.has_value());
    cr_assert(near(length(result.modifiedNormal.value()), 1.0));
    Vec3 modNormal = result.modifiedNormal.value();
    cr_assert(near(modNormal.x, 0.0) || std::abs(modNormal.x) < 0.01);
    cr_assert(near(modNormal.y, 1.0) || std::abs(modNormal.y - 1.0) < 0.01);
    cr_assert(near(modNormal.z, 0.0) || std::abs(modNormal.z) < 0.01);

    std::filesystem::remove("textures/test_normal_blue.ppm");
}
