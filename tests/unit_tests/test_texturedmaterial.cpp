/*
 * Project: Raytracer
 * File name: test_texturedmaterial.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for TexturedMaterial.
 */

#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <fstream>
#include <filesystem>
#include <cmath>

#include "TexturedMaterial.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"

static constexpr double EPS = 1e-9;

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return std::abs(a.x - b.x) < EPS && std::abs(a.y - b.y) < EPS && std::abs(a.z - b.z) < EPS;
}

static HitRecord makeHit(Vec3 point, Vec3 normal, double u, double v) {
    return HitRecord{1.0, point, normalize(normal), nullptr, true, {u, v}, Vec3(0,0,0), Vec3(0,0,0)};
}

Test(texturedmaterial, load_valid_ppm_no_throw) {
    cr_assert_no_throw(TexturedMaterial mat("textures/checkerboard.ppm"));
}

Test(texturedmaterial, load_invalid_file_throws) {
    std::ofstream bad("textures/bad.txt");
    bad << "not a ppm";
    bad.close();
    cr_assert_throw(TexturedMaterial mat("textures/bad.txt"), std::runtime_error);
    std::filesystem::remove("textures/bad.txt");
}

Test(texturedmaterial, scatter_returns_no_scattered_ray) {
    TexturedMaterial mat("textures/checkerboard.ppm");
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, 0.5, 0.5);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert_not(result.scatteredRay.has_value());
}

Test(texturedmaterial, scatter_returns_color_from_texture) {
    std::ofstream file("textures/test_2x2.ppm");
    file << "P3\n2 2\n255\n";
    file << "255 0 0 0 255 0\n";
    file << "0 0 255 255 255 255\n";
    file.close();
    
    TexturedMaterial mat("textures/test_2x2.ppm");
    
    HitRecord hit1 = makeHit({0, 0, 0}, {0, 1, 0}, 0.25, 0.25);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    ScatterResult result1 = mat.scatter(ray, hit1);
    cr_assert(vec3_near(result1.attenuation, Color{1.0, 0.0, 0.0}));
    
    std::filesystem::remove("textures/test_2x2.ppm");
}

Test(texturedmaterial, u_out_of_range_wraps) {
    std::ofstream file("textures/test_wrap.ppm");
    file << "P3\n2 2\n255\n";
    file << "255 0 0 0 255 0\n";
    file << "0 0 255 255 255 255\n";
    file.close();
    
    TexturedMaterial mat("textures/test_wrap.ppm");
    
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, 1.5, 0.25);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, Color{0.0, 1.0, 0.0}));
    
    std::filesystem::remove("textures/test_wrap.ppm");
}

Test(texturedmaterial, empty_texture_returns_magenta) {
    TexturedMaterial mat("nonexistent.ppm");
    HitRecord hit = makeHit({0, 0, 0}, {0, 1, 0}, 0.5, 0.5);
    Ray ray{{0, 0, 5}, {0, 0, -1}};
    
    ScatterResult result = mat.scatter(ray, hit);
    
    cr_assert(vec3_near(result.attenuation, Color{1.0, 0.0, 1.0}));
}
