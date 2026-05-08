/*
 * Project: Raytracer
 * File name: test_pointlight.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for PointLight.
 */

#include <criterion/criterion.h>
#include <cmath>
#include "PointLight.hpp"
#include "Color.hpp"

Test(pointlight, get_sample_returns_direction_toward_light) {
    PointLight light(Vec3{0, 0, 5}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.direction.x, 0.0, 1e-15);
    cr_assert_float_eq(sample.direction.y, 0.0, 1e-15);
    cr_assert_float_eq(sample.direction.z, 1.0, 1e-15);
}

Test(pointlight, get_sample_returns_normalized_direction) {
    PointLight light(Vec3{3, 4, 0}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    double len = std::sqrt(sample.direction.x * sample.direction.x +
                       sample.direction.y * sample.direction.y +
                       sample.direction.z * sample.direction.z);
    cr_assert_float_eq(len, 1.0, 1e-15);
}

Test(pointlight, get_sample_returns_correct_distance) {
    PointLight light(Vec3{0, 0, 10}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.distance, 10.0, 1e-15);
}

Test(pointlight, get_sample_distance_changes_with_hit_point) {
    PointLight light(Vec3{0, 0, 0}, Color{1, 1, 1}, 1.0);
    LightSample sample1 = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    LightSample sample2 = light.getSample(Vec3{0, 0, 10}, Vec3{0, 0, 1});
    cr_assert_float_neq(sample1.distance, sample2.distance, 1e-15);
}

Test(pointlight, get_sample_returns_color_with_quadratic_attenuation) {
    PointLight light(Vec3{0, 0, 2}, Color{1, 1, 1}, 1.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.color.x, 0.25, 1e-15);
    cr_assert_float_eq(sample.color.y, 0.25, 1e-15);
    cr_assert_float_eq(sample.color.z, 0.25, 1e-15);
}

Test(pointlight, get_sample_color_scaled_by_intensity) {
    PointLight light(Vec3{0, 0, 1}, Color{1, 1, 1}, 2.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.color.x, 2.0, 1e-15);
}

Test(pointlight, hit_point_affects_direction) {
    PointLight light(Vec3{0, 0, 10}, Color{1, 1, 1}, 1.0);
    LightSample sample1 = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    LightSample sample2 = light.getSample(Vec3{10, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_neq(sample1.direction.x, sample2.direction.x, 1e-15);
}

Test(pointlight, normal_parameter_not_used) {
    PointLight light(Vec3{0, 0, 10}, Color{1, 1, 1}, 1.0);
    LightSample sample1 = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    LightSample sample2 = light.getSample(Vec3{0, 0, 0}, Vec3{1, 0, 0});
    cr_assert_float_eq(sample1.color.x, sample2.color.x, 1e-15);
    cr_assert_float_eq(sample1.color.y, sample2.color.y, 1e-15);
    cr_assert_float_eq(sample1.color.z, sample2.color.z, 1e-15);
}

Test(pointlight, zero_intensity_returns_zero_color) {
    PointLight light(Vec3{0, 0, 1}, Color{1, 1, 1}, 0.0);
    LightSample sample = light.getSample(Vec3{0, 0, 0}, Vec3{0, 0, 1});
    cr_assert_float_eq(sample.color.x, 0.0, 1e-15);
    cr_assert_float_eq(sample.color.y, 0.0, 1e-15);
    cr_assert_float_eq(sample.color.z, 0.0, 1e-15);
}
