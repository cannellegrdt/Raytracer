/*
 * Project: Raytracer
 * File name: test_directionallight.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for DirectionalLight.
 */

#include <criterion/criterion.h>
#include <limits>
#include <cmath>
#include "DirectionalLight.hpp"
#include "Color.hpp"

Test(directionallight, get_sample_returns_normalized_direction) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    double len = std::sqrt(sample.direction.x * sample.direction.x +
                       sample.direction.y * sample.direction.y +
                       sample.direction.z * sample.direction.z);
    cr_assert_float_eq(len, 1.0, 1e-15);
}

Test(directionallight, get_sample_returns_opposite_direction) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.direction.x, 0.0, 1e-15);
    cr_assert_float_eq(sample.direction.y, 0.0, 1e-15);
    cr_assert_float_eq(sample.direction.z, 1.0, 1e-15);
}

Test(directionallight, get_sample_returns_color_scaled_by_intensity) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 0.5, 0.25}, 2.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.color.x, 2.0, 1e-15);
    cr_assert_float_eq(sample.color.y, 1.0, 1e-15);
    cr_assert_float_eq(sample.color.z, 0.5, 1e-15);
}

Test(directionallight, get_sample_returns_infinity_distance) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert(std::isinf(sample.distance));
}

Test(directionallight, hit_point_parameter_not_used) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 1.0);
    LightSample sample1 = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    LightSample sample2 = light.getSample(Vec3{100, 200, 300}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample1.direction.x, sample2.direction.x, 1e-15);
    cr_assert_float_eq(sample1.direction.y, sample2.direction.y, 1e-15);
    cr_assert_float_eq(sample1.direction.z, sample2.direction.z, 1e-15);
}

Test(directionallight, normal_parameter_not_used) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 1.0);
    LightSample sample1 = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    LightSample sample2 = light.getSample(Vec3{0, 0, 0}, Vec3{1, 0, 0});
    cr_assert_float_eq(sample1.color.x, sample2.color.x, 1e-15);
    cr_assert_float_eq(sample1.color.y, sample2.color.y, 1e-15);
    cr_assert_float_eq(sample1.color.z, sample2.color.z, 1e-15);
}

Test(directionallight, positive_z_direction_produces_negative_sample) {
    DirectionalLight light(Vec3{0, 0, 1}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.direction.z, -1.0, 1e-15);
}

Test(directionallight, zero_intensity_returns_zero_color) {
    DirectionalLight light(Vec3{0, 0, -1}, Color{1, 1, 1}, 0.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.color.x, 0.0, 1e-15);
    cr_assert_float_eq(sample.color.y, 0.0, 1e-15);
    cr_assert_float_eq(sample.color.z, 0.0, 1e-15);
}