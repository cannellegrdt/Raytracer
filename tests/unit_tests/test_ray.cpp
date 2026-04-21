/*
 * Project: Raytracer
 * File name: test_ray.cpp
 * Description: Criterion unit tests for Ray - parametric point evaluation.
 */

#include <criterion/criterion.h>
#include "Ray.hpp"

Test(ray, at_zero_returns_origin) {
    Ray r{{1, 2, 3}, {4, 5, 6}};
    Vec3 p = r.at(0.0);
    cr_assert_float_eq(p.x, 1.0, 1e-15);
    cr_assert_float_eq(p.y, 2.0, 1e-15);
    cr_assert_float_eq(p.z, 3.0, 1e-15);
}

Test(ray, at_one_returns_origin_plus_direction) {
    Ray r{{1, 2, 3}, {1, 0, 0}};
    Vec3 p = r.at(1.0);
    cr_assert_float_eq(p.x, 2.0, 1e-15);
    cr_assert_float_eq(p.y, 2.0, 1e-15);
    cr_assert_float_eq(p.z, 3.0, 1e-15);
}

Test(ray, at_parametric_t_scales_direction) {
    Ray r{{0, 0, 0}, {1, 2, 3}};
    Vec3 p = r.at(2.0);
    cr_assert_float_eq(p.x, 2.0, 1e-15);
    cr_assert_float_eq(p.y, 4.0, 1e-15);
    cr_assert_float_eq(p.z, 6.0, 1e-15);
}

Test(ray, at_negative_t_goes_backward) {
    Ray r{{5, 5, 5}, {1, 0, 0}};
    Vec3 p = r.at(-1.0);
    cr_assert_float_eq(p.x, 4.0, 1e-15);
    cr_assert_float_eq(p.y, 5.0, 1e-15);
    cr_assert_float_eq(p.z, 5.0, 1e-15);
}

Test(ray, at_fractional_t) {
    Ray r{{0, 0, 0}, {2, 4, 6}};
    Vec3 p = r.at(0.5);
    cr_assert_float_eq(p.x, 1.0, 1e-15);
    cr_assert_float_eq(p.y, 2.0, 1e-15);
    cr_assert_float_eq(p.z, 3.0, 1e-15);
}
