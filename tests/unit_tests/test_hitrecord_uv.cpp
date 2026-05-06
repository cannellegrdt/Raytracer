/*
 * Project: Raytracer
 * File name: test_hitrecord_uv.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for HitRecord UV coordinates.
 */

#include <criterion/criterion.h>
#include <cmath>

#include "../../include/HitRecord.hpp"
#include "../../include/Vec3.hpp"

static constexpr double EPS = 1e-9;

Test(hitrecord, has_u_v_fields) {
    HitRecord hit{1.0, {0, 0, 0}, {0, 1, 0}, nullptr, true, {0.3, 0.7}};
    
    cr_assert_float_eq(hit.UV.first, 0.3, EPS);
    cr_assert_float_eq(hit.UV.second, 0.7, EPS);
}

Test(hitrecord, default_uv_is_zero) {
    HitRecord hit{};
    
    cr_assert_float_eq(hit.UV.first, 0.0, EPS);
    cr_assert_float_eq(hit.UV.second, 0.0, EPS);
}

Test(hitrecord, uv_in_range_0_1) {
    HitRecord hit{1.0, {0, 0, 0}, {0, 1, 0}, nullptr, true, {0.0, 1.0}};
    
    cr_assert(hit.UV.first >= 0.0 && hit.UV.first <= 1.0);
    cr_assert(hit.UV.second >= 0.0 && hit.UV.second <= 1.0);
}
