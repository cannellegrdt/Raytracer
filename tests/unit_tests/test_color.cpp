/*
 * Project: Raytracer
 * File name: test_color.cpp
 * Description: Criterion unit tests for Color - clamp() and toPPMByte() conversions.
 */

#include <criterion/criterion.h>
#include "Color.hpp"

Test(color, clamp_within_range_unchanged) {
    Color c(0.5, 0.3, 0.8);
    Color r = clamp(c);
    cr_assert_float_eq(r.x, 0.5, 1e-15);
    cr_assert_float_eq(r.y, 0.3, 1e-15);
    cr_assert_float_eq(r.z, 0.8, 1e-15);
}

Test(color, clamp_above_max_saturates_to_one) {
    Color c(1.5, 2.0, 100.0);
    Color r = clamp(c);
    cr_assert_float_eq(r.x, 1.0, 1e-15);
    cr_assert_float_eq(r.y, 1.0, 1e-15);
    cr_assert_float_eq(r.z, 1.0, 1e-15);
}

Test(color, clamp_below_min_saturates_to_zero) {
    Color c(-0.5, -1.0, -100.0);
    Color r = clamp(c);
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 0.0, 1e-15);
    cr_assert_float_eq(r.z, 0.0, 1e-15);
}

Test(color, clamp_boundary_values_unchanged) {
    Color c(0.0, 1.0, 0.5);
    Color r = clamp(c);
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 1.0, 1e-15);
    cr_assert_float_eq(r.z, 0.5, 1e-15);
}

Test(color, clamp_custom_range) {
    Color c(0.5, 1.5, -0.5);
    Color r = clamp(c, 0.2, 0.8);
    cr_assert_float_eq(r.x, 0.5, 1e-15);
    cr_assert_float_eq(r.y, 0.8, 1e-15);
    cr_assert_float_eq(r.z, 0.2, 1e-15);
}

Test(color, to_ppm_byte_zero_is_zero) {
    cr_assert_eq(toPPMByte(0.0), 0);
}

Test(color, to_ppm_byte_one_is_255) {
    cr_assert_eq(toPPMByte(1.0), 255);
}

Test(color, to_ppm_byte_half_is_127_or_128) {
    int val = toPPMByte(0.5);
    cr_assert(val == 127 || val == 128,
        "Expected toPPMByte(0.5) to be 127 or 128, got %d", val);
}

Test(color, to_ppm_byte_negative_clamped_to_zero) {
    cr_assert_eq(toPPMByte(-1.0), 0);
}

Test(color, to_ppm_byte_above_one_clamped_to_255) {
    cr_assert_eq(toPPMByte(2.0), 255);
}

Test(color, to_ppm_byte_quarter_is_in_range) {
    int val = toPPMByte(0.25);
    cr_assert(val >= 63 && val <= 64,
        "Expected toPPMByte(0.25) around 63-64, got %d", val);
}
