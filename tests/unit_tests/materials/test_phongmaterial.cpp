/*
 * Project: Raytracer
 * File name: test_phongmaterial.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for PhongMaterial.
 */

#include <criterion/criterion.h>
#include <cmath>
#include "PhongMaterial.hpp"
#include "SpecularParams.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"
#include "Vec3.hpp"

static HitRecord makeHit(const Vec3 &point, const Vec3 &normal) {
    HitRecord hit;
    hit.t = 0.0;
    hit.point = point;
    hit.normal = normal;
    hit.material = nullptr;
    hit.frontFace = true;
    return hit;
}

Test(phongmaterial, scatter_returns_diffuse_color) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(1.0, 1.0, 1.0);
    PhongMaterial mat(diffuse, specular, 32.0);
    Ray ray(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));
    HitRecord hit = makeHit(Vec3(0.0, 0.0, 5.0), Vec3(0.0, 0.0, -1.0));
    
    ScatterResult result = mat.scatter(ray, hit);
    cr_assert_float_eq(result.attenuation.x, 0.8, 1e-15);
    cr_assert_float_eq(result.attenuation.y, 0.2, 1e-15);
    cr_assert_float_eq(result.attenuation.z, 0.2, 1e-15);
}

Test(phongmaterial, scatter_returns_no_scattered_ray) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(1.0, 1.0, 1.0);
    PhongMaterial mat(diffuse, specular, 32.0);
    Ray ray(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));
    HitRecord hit = makeHit(Vec3(0.0, 0.0, 5.0), Vec3(0.0, 0.0, -1.0));
    
    ScatterResult result = mat.scatter(ray, hit);
    cr_assert(!result.scatteredRay.has_value());
}

Test(phongmaterial, getSpecular_returns_params) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(1.0, 1.0, 1.0);
    double shininess = 32.0;
    PhongMaterial mat(diffuse, specular, shininess);
    
    auto params = mat.getSpecular();
    cr_assert(params.has_value());
    cr_assert_float_eq(params->ks.x, 1.0, 1e-15);
    cr_assert_float_eq(params->ks.y, 1.0, 1e-15);
    cr_assert_float_eq(params->ks.z, 1.0, 1e-15);
    cr_assert_float_eq(params->shininess, 32.0, 1e-15);
}

Test(phongmaterial, getSpecular_different_specular_color) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(0.5, 0.5, 1.0);
    PhongMaterial mat(diffuse, specular, 64.0);
    
    auto params = mat.getSpecular();
    cr_assert(params.has_value());
    cr_assert_float_eq(params->ks.x, 0.5, 1e-15);
    cr_assert_float_eq(params->ks.y, 0.5, 1e-15);
    cr_assert_float_eq(params->ks.z, 1.0, 1e-15);
}

Test(phongmaterial, getSpecular_high_shininess) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(1.0, 1.0, 1.0);
    PhongMaterial mat(diffuse, specular, 128.0);
    
    auto params = mat.getSpecular();
    cr_assert(params.has_value());
    cr_assert_float_eq(params->shininess, 128.0, 1e-15);
}

Test(phongmaterial, getSpecular_low_shininess) {
    Color diffuse(0.8, 0.2, 0.2);
    Color specular(1.0, 1.0, 1.0);
    PhongMaterial mat(diffuse, specular, 8.0);
    
    auto params = mat.getSpecular();
    cr_assert(params.has_value());
    cr_assert_float_eq(params->shininess, 8.0, 1e-15);
}
