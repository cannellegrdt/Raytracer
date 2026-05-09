/*
 * Project: Raytracer
 * File name: test_aabb.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for AABB - construction, merge, intersection.
 */

#include <criterion/criterion.h>
#include "AABB.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"

Test(aabb, default_construction) {
    AABB box;
    cr_assert_float_eq(box.min.x, 0.0, 1e-15);
    cr_assert_float_eq(box.min.y, 0.0, 1e-15);
    cr_assert_float_eq(box.min.z, 0.0, 1e-15);
    cr_assert_float_eq(box.max.x, 0.0, 1e-15);
    cr_assert_float_eq(box.max.y, 0.0, 1e-15);
    cr_assert_float_eq(box.max.z, 0.0, 1e-15);
}

Test(aabb, parametric_construction) {
    Vec3 min(1.0, 2.0, 3.0);
    Vec3 max(4.0, 5.0, 6.0);
    AABB box(min, max);
    cr_assert_float_eq(box.min.x, 1.0, 1e-15);
    cr_assert_float_eq(box.min.y, 2.0, 1e-15);
    cr_assert_float_eq(box.min.z, 3.0, 1e-15);
    cr_assert_float_eq(box.max.x, 4.0, 1e-15);
    cr_assert_float_eq(box.max.y, 5.0, 1e-15);
    cr_assert_float_eq(box.max.z, 6.0, 1e-15);
}

Test(aabb, empty_box) {
    AABB box = AABB::empty();
    cr_assert(std::isinf(box.min.x) && box.min.x > 0);
    cr_assert(std::isinf(box.min.y) && box.min.y > 0);
    cr_assert(std::isinf(box.min.z) && box.min.z > 0);
    cr_assert(std::isinf(box.max.x) && box.max.x < 0);
    cr_assert(std::isinf(box.max.y) && box.max.y < 0);
    cr_assert(std::isinf(box.max.z) && box.max.z < 0);
}

Test(aabb, infinite_box) {
    AABB box = AABB::infinite();
    cr_assert(std::isinf(box.min.x) && box.min.x < 0);
    cr_assert(std::isinf(box.min.y) && box.min.y < 0);
    cr_assert(std::isinf(box.min.z) && box.min.z < 0);
    cr_assert(std::isinf(box.max.x) && box.max.x > 0);
    cr_assert(std::isinf(box.max.y) && box.max.y > 0);
    cr_assert(std::isinf(box.max.z) && box.max.z > 0);
}

Test(aabb, is_infinite_false) {
    AABB box(Vec3(0,0,0), Vec3(1,1,1));
    cr_assert_eq(box.isInfinite(), false);
}

Test(aabb, is_infinite_true_x) {
    AABB box(Vec3(-std::numeric_limits<double>::infinity(), 0, 0),
             Vec3(std::numeric_limits<double>::infinity(), 1, 1));
    cr_assert_eq(box.isInfinite(), true);
}

Test(aabb, is_infinite_true_y) {
    AABB box(Vec3(0, -std::numeric_limits<double>::infinity(), 0),
             Vec3(1, std::numeric_limits<double>::infinity(), 1));
    cr_assert_eq(box.isInfinite(), true);
}

Test(aabb, is_infinite_true_z) {
    AABB box(Vec3(0, 0, -std::numeric_limits<double>::infinity()),
             Vec3(1, 1, std::numeric_limits<double>::infinity()));
    cr_assert_eq(box.isInfinite(), true);
}

Test(aabb, centroid) {
    AABB box(Vec3(1, 2, 3), Vec3(3, 4, 5));
    Vec3 c = box.centroid();
    cr_assert_float_eq(c.x, 2.0, 1e-15);
    cr_assert_float_eq(c.y, 3.0, 1e-15);
    cr_assert_float_eq(c.z, 4.0, 1e-15);
}

Test(aabb, merge) {
    AABB a(Vec3(1, 1, 1), Vec3(2, 2, 2));
    AABB b(Vec3(0, 0, 0), Vec3(3, 3, 3));
    AABB merged = AABB::merge(a, b);
    cr_assert_float_eq(merged.min.x, 0.0, 1e-15);
    cr_assert_float_eq(merged.min.y, 0.0, 1e-15);
    cr_assert_float_eq(merged.min.z, 0.0, 1e-15);
    cr_assert_float_eq(merged.max.x, 3.0, 1e-15);
    cr_assert_float_eq(merged.max.y, 3.0, 1e-15);
    cr_assert_float_eq(merged.max.z, 3.0, 1e-15);
}

Test(aabb, intersect_inside) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(0, 0, -5), Vec3(0, 0, 1));
    double tMin = -1.0, tMax = 10.0;
    cr_assert_eq(box.intersect(ray, tMin, tMax), true);
    cr_assert_float_eq(tMin, 4.0, 1e-15);
    cr_assert_float_eq(tMax, 6.0, 1e-15);
}

Test(aabb, intersect_outside) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(2, 0, 0), Vec3(1, 0, 0));
    double tMin = -1.0, tMax = 10.0;
    cr_assert_eq(box.intersect(ray, tMin, tMax), false);
}

Test(aabb, intersect_near_far) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(0, 0, -5), Vec3(0, 0, 1));
    double tMin = -1.0, tMax = 10.0;
    cr_assert_eq(box.intersect(ray, tMin, tMax), true);
}

Test(aabb, intersect_no_overlap) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(0, 0, -5), Vec3(0, 0, 1));
    double tMin = 7.0, tMax = 10.0;
    cr_assert_eq(box.intersect(ray, tMin, tMax), false);
}

Test(aabb, intersect_inside_out) {
    AABB box(Vec3(-1, -1, -1), Vec3(1, 1, 1));
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, 1));
    double tMin = -2.0, tMax = 10.0;
    cr_assert_eq(box.intersect(ray, tMin, tMax), true);
    cr_assert_float_eq(tMin, 0.0, 1e-15);
    cr_assert_float_eq(tMax, 1.0, 1e-15);
}