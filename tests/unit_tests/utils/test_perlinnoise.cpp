/*
 * Project: Raytracer
 * File name: test_perlinnoise.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for PerlinNoise - noise generation, fractal Brownian motion, and helper functions.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <optional>

#include "PerlinNoise.hpp"
#include "Vec3.hpp"

static constexpr double EPS = 1e-9;

Test(perlinnoise, default_constructor_creates_valid_instance) {
    PerlinNoise noise;
    cr_assert(true);
}

Test(perlinnoise, constructor_with_seed_creates_valid_instance) {
    PerlinNoise noise(42);
    cr_assert(true);
}

Test(perlinnoise, same_seed_produces_same_noise) {
    PerlinNoise noise1(123);
    PerlinNoise noise2(123);
    Vec3 point(1.5, 2.5, 3.5);
    cr_assert_float_eq(noise1.noise(point), noise2.noise(point), EPS);
}

Test(perlinnoise, different_seed_produces_different_noise) {
    PerlinNoise noise1(1);
    PerlinNoise noise2(2);
    Vec3 point(1.5, 2.5, 3.5);
    cr_assert_float_neq(noise1.noise(point), noise2.noise(point), EPS);
}

Test(perlinnoise, noise_returns_value_in_zero_one_range) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double val = noise.noise(point);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, noise_is_deterministic) {
    PerlinNoise noise;
    Vec3 point(4.0, 5.0, 6.0);
    double val1 = noise.noise(point);
    double val2 = noise.noise(point);
    cr_assert_float_eq(val1, val2, EPS);
}

Test(perlinnoise, noise_at_integer_points) {
    PerlinNoise noise;
    Vec3 point(1.0, 2.0, 3.0);
    double val = noise.noise(point);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, noise_at_origin) {
    PerlinNoise noise;
    Vec3 point(0.0, 0.0, 0.0);
    double val = noise.noise(point);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, noise_at_negative_coordinates) {
    PerlinNoise noise;
    Vec3 point(-1.5, -2.5, -3.5);
    double val = noise.noise(point);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, fractal_returns_value_in_zero_one_range) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double val = noise.fractal(point, 4);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, fractal_with_one_octave_equals_noise) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double noiseVal = noise.noise(point);
    double fractalVal = noise.fractal(point, 1);
    cr_assert_float_eq(noiseVal, fractalVal, EPS);
}

Test(perlinnoise, fractal_is_deterministic) {
    PerlinNoise noise;
    Vec3 point(4.0, 5.0, 6.0);
    double val1 = noise.fractal(point, 3);
    double val2 = noise.fractal(point, 3);
    cr_assert_float_eq(val1, val2, EPS);
}

Test(perlinnoise, fractal_with_zero_octaves_returns_zero) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double val = noise.fractal(point, 0);
    cr_assert_float_eq(val, 0.0, EPS);
}

Test(perlinnoise, fractal_with_multiple_octaves) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double val = noise.fractal(point, 6);
    cr_assert(val >= 0.0 && val <= 1.0);
}

Test(perlinnoise, noise_continuity_small_delta) {
    PerlinNoise noise;
    Vec3 point(1.5, 2.5, 3.5);
    double val1 = noise.noise(point);
    double val2 = noise.noise(point + Vec3(0.001, 0.001, 0.001));
    cr_assert(std::abs(val1 - val2) < 0.1);
}
