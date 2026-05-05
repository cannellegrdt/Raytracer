/*
 * Project: Raytracer
 * File name: test_mat4.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for Mat4 - multiplication, transpose, inverse, transform helpers.
 */

#include <criterion/criterion.h>
#include <cmath>
#include "../../include/Mat4.hpp"

static constexpr double EPS = 1e-9;

static bool near(double a, double b) {
    return std::abs(a - b) < EPS;
}

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

Test(mat4, identity_has_ones_on_diagonal) {
    Mat4 m = identity4();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cr_assert(near(m.m[i][j], (i == j) ? 1.0 : 0.0));
}

Test(mat4, identity_times_vector_returns_same) {
    Mat4 m = identity4();
    Vec3 v{1, 2, 3};
    cr_assert(vec3_near(transformPoint(m, v), v));
    cr_assert(vec3_near(transformDirection(m, v), v));
}

Test(mat4, translate_moves_point) {
    Mat4 m = translate(1.0, 2.0, 3.0);
    Vec3 p{0, 0, 0};
    Vec3 result = transformPoint(m, p);
    cr_assert(vec3_near(result, {1, 2, 3}));
}

Test(mat4, translate_does_not_affect_direction) {
    Mat4 m = translate(1.0, 2.0, 3.0);
    Vec3 d{1, 0, 0};
    Vec3 result = transformDirection(m, d);
    cr_assert(vec3_near(result, {1, 0, 0}));
}

Test(mat4, scale_changes_point) {
    Mat4 m = scale(2.0, 3.0, 4.0);
    Vec3 p{1, 1, 1};
    Vec3 result = transformPoint(m, p);
    cr_assert(vec3_near(result, {2, 3, 4}));
}

Test(mat4, scale_changes_direction) {
    Mat4 m = scale(2.0, 3.0, 1.0);
    Vec3 d{1, 0, 0};
    Vec3 result = transformDirection(m, d);
    cr_assert(vec3_near(result, {2, 0, 0}));
}

Test(mat4, rotateX_90_maps_y_to_z) {
    Mat4 m = rotateX4(M_PI / 2.0);
    Vec3 r = transformDirection(m, {0, 1, 0});
    cr_assert(vec3_near(r, {0, 0, 1}));
}

Test(mat4, rotateX_90_maps_z_to_neg_y) {
    Mat4 m = rotateX4(M_PI / 2.0);
    Vec3 r = transformDirection(m, {0, 0, 1});
    cr_assert(vec3_near(r, {0, -1, 0}));
}

Test(mat4, rotateY_90_maps_x_to_neg_z) {
    Mat4 m = rotateY4(M_PI / 2.0);
    Vec3 r = transformDirection(m, {1, 0, 0});
    cr_assert(vec3_near(r, {0, 0, -1}));
}

Test(mat4, rotateZ_90_maps_x_to_y) {
    Mat4 m = rotateZ4(M_PI / 2.0);
    Vec3 r = transformDirection(m, {1, 0, 0});
    cr_assert(vec3_near(r, {0, 1, 0}));
}

Test(mat4, matrix_multiplication_associative) {
    Mat4 a = rotateX4(0.5);
    Mat4 b = rotateY4(0.3);
    Mat4 c = rotateZ4(0.7);
    Mat4 left = a * (b * c);
    Mat4 right = (a * b) * c;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cr_assert(near(left.m[i][j], right.m[i][j]));
}

Test(mat4, transpose_twice_returns_original) {
    Mat4 m = translate(1, 2, 3) * rotateX4(0.5);
    Mat4 t = m.transpose().transpose();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cr_assert(near(m.m[i][j], t.m[i][j]));
}

Test(mat4, inverse_of_identity_is_identity) {
    Mat4 m = identity4();
    Mat4 inv = m.inverse();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cr_assert(near(inv.m[i][j], (i == j) ? 1.0 : 0.0));
}

Test(mat4, inverse_of_translate_negates_offset) {
    Mat4 m = translate(1.0, 2.0, 3.0);
    Mat4 inv = m.inverse();
    Vec3 p{0, 0, 0};
    Vec3 result = transformPoint(inv, p);
    cr_assert(vec3_near(result, {-1, -2, -3}));
}

Test(mat4, inverse_of_rotateX_restores_vector) {
    Mat4 m = rotateX4(M_PI / 3.0);
    Mat4 inv = m.inverse();
    Vec3 v{0, 1, 0};
    Vec3 r = transformDirection(inv, transformDirection(m, v));
    cr_assert(vec3_near(r, v));
}

Test(mat4, inverse_times_original_gives_identity) {
    Mat4 m = translate(1, 2, 3) * rotateY4(0.7) * scale(2, 1, 3);
    Mat4 inv = m.inverse();
    Mat4 product = m * inv;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cr_assert(near(product.m[i][j], (i == j) ? 1.0 : 0.0));
}

Test(mat4, transformPoint_applies_translation) {
    Mat4 m = translate(5.0, 0.0, 0.0);
    Vec3 p{1, 2, 3};
    Vec3 result = transformPoint(m, p);
    cr_assert(vec3_near(result, {6, 2, 3}));
}

Test(mat4, transformDirection_ignores_translation) {
    Mat4 m = translate(5.0, 10.0, 15.0);
    Vec3 d{1, 0, 0};
    Vec3 result = transformDirection(m, d);
    cr_assert(vec3_near(result, {1, 0, 0}));
}

Test(mat4, lookAt_creates_valid_view_matrix) {
    Vec3 eye{0, 0, 5};
    Vec3 center{0, 0, 0};
    Vec3 up{0, 1, 0};
    Mat4 view = lookAt(eye, center, up);
    Vec3 origin = transformPoint(view, eye);
    cr_assert(vec3_near(origin, {0, 0, 0}));
}

Test(mat4, perspective_creates_valid_projection) {
    Mat4 proj = perspective(M_PI / 4.0, 1.0, 0.1, 100.0);
    cr_assert_not(near(proj.m[2][3], 0.0));
    cr_assert(near(proj.m[3][2], -1.0));
}
