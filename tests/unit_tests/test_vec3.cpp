/*
 * Project: Raytracer
 * File name: test_vec3.cpp
 * Description: Criterion unit tests for Vec3 - construction, operators, dot, cross, length, normalize.
 */

#include <criterion/criterion.h>
#include "Vec3.hpp"

Test(vec3, default_construction_is_zero) {
    Vec3 v;
    cr_assert_float_eq(v.x, 0.0, 1e-15);
    cr_assert_float_eq(v.y, 0.0, 1e-15);
    cr_assert_float_eq(v.z, 0.0, 1e-15);
}

Test(vec3, parametric_construction) {
    Vec3 v(1.0, 2.0, 3.0);
    cr_assert_float_eq(v.x, 1.0, 1e-15);
    cr_assert_float_eq(v.y, 2.0, 1e-15);
    cr_assert_float_eq(v.z, 3.0, 1e-15);
}

Test(vec3, subscript_read) {
    Vec3 v(4.0, 5.0, 6.0);
    cr_assert_float_eq(v[0], 4.0, 1e-15);
    cr_assert_float_eq(v[1], 5.0, 1e-15);
    cr_assert_float_eq(v[2], 6.0, 1e-15);
}

Test(vec3, subscript_write) {
    Vec3 v;
    v[0] = 7.0;
    v[1] = 8.0;
    v[2] = 9.0;
    cr_assert_float_eq(v.x, 7.0, 1e-15);
    cr_assert_float_eq(v.y, 8.0, 1e-15);
    cr_assert_float_eq(v.z, 9.0, 1e-15);
}

Test(vec3, addition) {
    Vec3 a(1, 2, 3), b(4, 5, 6);
    Vec3 r = a + b;
    cr_assert_float_eq(r.x, 5.0, 1e-15);
    cr_assert_float_eq(r.y, 7.0, 1e-15);
    cr_assert_float_eq(r.z, 9.0, 1e-15);
}

Test(vec3, subtraction) {
    Vec3 a(5, 7, 9), b(1, 2, 3);
    Vec3 r = a - b;
    cr_assert_float_eq(r.x, 4.0, 1e-15);
    cr_assert_float_eq(r.y, 5.0, 1e-15);
    cr_assert_float_eq(r.z, 6.0, 1e-15);
}

Test(vec3, component_wise_multiplication) {
    Vec3 a(2, 3, 4), b(5, 6, 7);
    Vec3 r = a * b;
    cr_assert_float_eq(r.x, 10.0, 1e-15);
    cr_assert_float_eq(r.y, 18.0, 1e-15);
    cr_assert_float_eq(r.z, 28.0, 1e-15);
}

Test(vec3, component_wise_division) {
    Vec3 a(10, 12, 14), b(2, 3, 7);
    Vec3 r = a / b;
    cr_assert_float_eq(r.x, 5.0, 1e-15);
    cr_assert_float_eq(r.y, 4.0, 1e-15);
    cr_assert_float_eq(r.z, 2.0, 1e-15);
}

Test(vec3, division_by_zero_component_yields_zero) {
    Vec3 a(10, 12, 14);
    Vec3 b(0, 0, 0);
    Vec3 r = a / b;
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 0.0, 1e-15);
    cr_assert_float_eq(r.z, 0.0, 1e-15);
}

Test(vec3, unary_negation) {
    Vec3 v(1, -2, 3);
    Vec3 r = -v;
    cr_assert_float_eq(r.x, -1.0, 1e-15);
    cr_assert_float_eq(r.y,  2.0, 1e-15);
    cr_assert_float_eq(r.z, -3.0, 1e-15);
}

Test(vec3, scalar_left_add) {
    Vec3 v(1, 2, 3);
    Vec3 r = 10.0 + v;
    cr_assert_float_eq(r.x, 11.0, 1e-15);
    cr_assert_float_eq(r.y, 12.0, 1e-15);
    cr_assert_float_eq(r.z, 13.0, 1e-15);
}

Test(vec3, scalar_right_add) {
    Vec3 v(1, 2, 3);
    Vec3 r = v + 10.0;
    cr_assert_float_eq(r.x, 11.0, 1e-15);
    cr_assert_float_eq(r.y, 12.0, 1e-15);
    cr_assert_float_eq(r.z, 13.0, 1e-15);
}

Test(vec3, scalar_left_subtract) {
    Vec3 v(1, 2, 3);
    Vec3 r = 10.0 - v;
    cr_assert_float_eq(r.x, 9.0, 1e-15);
    cr_assert_float_eq(r.y, 8.0, 1e-15);
    cr_assert_float_eq(r.z, 7.0, 1e-15);
}

Test(vec3, scalar_right_subtract) {
    Vec3 v(1, 2, 3);
    Vec3 r = v - 1.0;
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 1.0, 1e-15);
    cr_assert_float_eq(r.z, 2.0, 1e-15);
}

Test(vec3, scalar_left_multiply) {
    Vec3 v(1, 2, 3);
    Vec3 r = 3.0 * v;
    cr_assert_float_eq(r.x, 3.0, 1e-15);
    cr_assert_float_eq(r.y, 6.0, 1e-15);
    cr_assert_float_eq(r.z, 9.0, 1e-15);
}

Test(vec3, scalar_right_multiply) {
    Vec3 v(1, 2, 3);
    Vec3 r = v * 3.0;
    cr_assert_float_eq(r.x, 3.0, 1e-15);
    cr_assert_float_eq(r.y, 6.0, 1e-15);
    cr_assert_float_eq(r.z, 9.0, 1e-15);
}

Test(vec3, scalar_divide) {
    Vec3 v(6, 9, 12);
    Vec3 r = v / 3.0;
    cr_assert_float_eq(r.x, 2.0, 1e-15);
    cr_assert_float_eq(r.y, 3.0, 1e-15);
    cr_assert_float_eq(r.z, 4.0, 1e-15);
}

Test(vec3, plus_equals) {
    Vec3 v(1, 2, 3);
    v += Vec3(4, 5, 6);
    cr_assert_float_eq(v.x, 5.0, 1e-15);
    cr_assert_float_eq(v.y, 7.0, 1e-15);
    cr_assert_float_eq(v.z, 9.0, 1e-15);
}

Test(vec3, minus_equals) {
    Vec3 v(5, 7, 9);
    v -= Vec3(1, 2, 3);
    cr_assert_float_eq(v.x, 4.0, 1e-15);
    cr_assert_float_eq(v.y, 5.0, 1e-15);
    cr_assert_float_eq(v.z, 6.0, 1e-15);
}

Test(vec3, times_equals) {
    Vec3 v(2, 3, 4);
    v *= Vec3(5, 6, 7);
    cr_assert_float_eq(v.x, 10.0, 1e-15);
    cr_assert_float_eq(v.y, 18.0, 1e-15);
    cr_assert_float_eq(v.z, 28.0, 1e-15);
}

Test(vec3, divide_equals) {
    Vec3 v(10, 12, 14);
    v /= Vec3(2, 3, 7);
    cr_assert_float_eq(v.x, 5.0, 1e-15);
    cr_assert_float_eq(v.y, 4.0, 1e-15);
    cr_assert_float_eq(v.z, 2.0, 1e-15);
}

Test(vec3, divide_equals_by_zero_yields_zero) {
    Vec3 v(10, 12, 14);
    v /= Vec3(0, 0, 0);
    cr_assert_float_eq(v.x, 0.0, 1e-15);
    cr_assert_float_eq(v.y, 0.0, 1e-15);
    cr_assert_float_eq(v.z, 0.0, 1e-15);
}

Test(vec3, dot_product) {
    Vec3 a(1, 2, 3), b(4, 5, 6);
    cr_assert_float_eq(dot(a, b), 32.0, 1e-15);
}

Test(vec3, dot_perpendicular_is_zero) {
    Vec3 a(1, 0, 0), b(0, 1, 0);
    cr_assert_float_eq(dot(a, b), 0.0, 1e-15);
}

Test(vec3, dot_self_is_length_squared) {
    Vec3 v(3, 4, 0);
    cr_assert_float_eq(dot(v, v), 25.0, 1e-15);
}

Test(vec3, cross_product_standard_basis) {
    Vec3 x(1, 0, 0), y(0, 1, 0);
    Vec3 r = cross(x, y);
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 0.0, 1e-15);
    cr_assert_float_eq(r.z, 1.0, 1e-15);
}

Test(vec3, cross_anticommutative) {
    Vec3 a(1, 2, 3), b(4, 5, 6);
    Vec3 r1 = cross(a, b);
    Vec3 r2 = cross(b, a);
    cr_assert_float_eq(r1.x, -r2.x, 1e-15);
    cr_assert_float_eq(r1.y, -r2.y, 1e-15);
    cr_assert_float_eq(r1.z, -r2.z, 1e-15);
}

Test(vec3, cross_parallel_vectors_is_zero) {
    Vec3 a(1, 0, 0), b(2, 0, 0);
    Vec3 r = cross(a, b);
    cr_assert_float_eq(r.x, 0.0, 1e-15);
    cr_assert_float_eq(r.y, 0.0, 1e-15);
    cr_assert_float_eq(r.z, 0.0, 1e-15);
}

Test(vec3, length_unit_x) {
    cr_assert_float_eq(length(Vec3(1, 0, 0)), 1.0, 1e-15);
}

Test(vec3, length_3_4_0_is_5) {
    cr_assert_float_eq(length(Vec3(3, 4, 0)), 5.0, 1e-15);
}

Test(vec3, length_zero_vector) {
    cr_assert_float_eq(length(Vec3(0, 0, 0)), 0.0, 1e-15);
}

Test(vec3, normalize_produces_unit_vector) {
    Vec3 n = normalize(Vec3(3, 4, 0));
    cr_assert_float_eq(length(n), 1.0, 1e-10);
    cr_assert_float_eq(n.x, 0.6, 1e-10);
    cr_assert_float_eq(n.y, 0.8, 1e-10);
    cr_assert_float_eq(n.z, 0.0, 1e-10);
}

Test(vec3, normalize_already_unit_unchanged) {
    Vec3 n = normalize(Vec3(1, 0, 0));
    cr_assert_float_eq(n.x, 1.0, 1e-15);
    cr_assert_float_eq(n.y, 0.0, 1e-15);
    cr_assert_float_eq(n.z, 0.0, 1e-15);
}

Test(vec3, normalize_zero_vector_returns_zero) {
    Vec3 n = normalize(Vec3(0, 0, 0));
    cr_assert_float_eq(n.x, 0.0, 1e-15);
    cr_assert_float_eq(n.y, 0.0, 1e-15);
    cr_assert_float_eq(n.z, 0.0, 1e-15);
}
