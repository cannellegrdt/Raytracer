/*
 * Project: Raytracer
 * File name: test_mat3.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for Mat3 - multiply, transpose, rotateX/Y/Z factory functions.
 */

#include <criterion/criterion.h>
#include <cmath>
#include "Mat3.hpp"

static constexpr double EPS = 1e-10;

static bool vec3_near(const Vec3 &a, const Vec3 &b) {
    return std::abs(a.x - b.x) < EPS &&
           std::abs(a.y - b.y) < EPS &&
           std::abs(a.z - b.z) < EPS;
}

Test(mat3, rotateX_zero_is_identity) {
    Vec3 v{1, 2, 3};
    cr_assert(vec3_near(rotateX(0.0) * v, v));
}

Test(mat3, rotateX_90_maps_y_to_z) {
    Vec3 r = rotateX(M_PI / 2.0) * Vec3{0, 1, 0};
    cr_assert(vec3_near(r, {0, 0, 1}));
}

Test(mat3, rotateX_90_maps_z_to_neg_y) {
    Vec3 r = rotateX(M_PI / 2.0) * Vec3{0, 0, 1};
    cr_assert(vec3_near(r, {0, -1, 0}));
}

Test(mat3, rotateX_pi_flips_y_and_z) {
    Vec3 r = rotateX(M_PI) * Vec3{0, 1, 0};
    cr_assert(vec3_near(r, {0, -1, 0}));
}

Test(mat3, rotateY_zero_is_identity) {
    Vec3 v{1, 2, 3};
    cr_assert(vec3_near(rotateY(0.0) * v, v));
}

Test(mat3, rotateY_90_maps_x_to_neg_z) {
    Vec3 r = rotateY(M_PI / 2.0) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {0, 0, -1}));
}

Test(mat3, rotateY_pi_flips_x_and_z) {
    Vec3 r = rotateY(M_PI) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {-1, 0, 0}));
}

Test(mat3, rotateZ_zero_is_identity) {
    Vec3 v{1, 2, 3};
    cr_assert(vec3_near(rotateZ(0.0) * v, v));
}

Test(mat3, rotateZ_90_maps_x_to_y) {
    Vec3 r = rotateZ(M_PI / 2.0) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {0, 1, 0}));
}

Test(mat3, rotateZ_pi_flips_x) {
    Vec3 r = rotateZ(M_PI) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {-1, 0, 0}));
}

Test(mat3, transpose_of_rotateX_is_inverse) {
    Mat3 m = rotateX(1.3);
    Vec3 v{1, 2, 3};
    cr_assert(vec3_near(m.transpose() * (m * v), v));
}

Test(mat3, transpose_of_rotateY_is_inverse) {
    Mat3 m = rotateY(0.9);
    Vec3 v{-1, 0, 4};
    cr_assert(vec3_near(m.transpose() * (m * v), v));
}

Test(mat3, double_transpose_is_identity) {
    Mat3 m = rotateZ(0.7);
    Vec3 v{2, -3, 1};
    cr_assert(vec3_near(m.transpose().transpose() * v, m * v));
}

Test(mat3, mat_mul_with_transpose_is_identity) {
    Mat3 m = rotateX(1.2) * rotateY(0.5);
    Mat3 inv = m.transpose();
    Vec3 v{3, -1, 2};
    cr_assert(vec3_near((m * inv) * v, v));
}

Test(mat3, mat_mul_compose_rotations) {
    Mat3 half = rotateZ(M_PI / 2.0);
    Vec3 r = (half * half) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {-1, 0, 0}));
}

Test(mat3, vec_mul_preserves_x_axis_for_rotateX) {
    Vec3 r = rotateX(1.5) * Vec3{1, 0, 0};
    cr_assert(vec3_near(r, {1, 0, 0}));
}

Test(mat3, vec_mul_preserves_y_axis_for_rotateY) {
    Vec3 r = rotateY(1.5) * Vec3{0, 1, 0};
    cr_assert(vec3_near(r, {0, 1, 0}));
}

Test(mat3, vec_mul_preserves_z_axis_for_rotateZ) {
    Vec3 r = rotateZ(1.5) * Vec3{0, 0, 1};
    cr_assert(vec3_near(r, {0, 0, 1}));
}
