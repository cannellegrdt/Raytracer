/*
 * Project: Raytracer
 * File name: test_hitrecord.cpp
 * Description: Criterion unit tests for HitRecord - field storage and face orientation flag.
 */

#include <criterion/criterion.h>
#include "HitRecord.hpp"

Test(hitrecord, fields_stored_correctly) {
    Vec3 pt(1, 2, 3);
    Vec3 n(0, 1, 0);
    HitRecord h{2.5, pt, n, nullptr, true};

    cr_assert_float_eq(h.t, 2.5, 1e-15);
    cr_assert_float_eq(h.point.x, 1.0, 1e-15);
    cr_assert_float_eq(h.point.y, 2.0, 1e-15);
    cr_assert_float_eq(h.point.z, 3.0, 1e-15);
    cr_assert_float_eq(h.normal.x, 0.0, 1e-15);
    cr_assert_float_eq(h.normal.y, 1.0, 1e-15);
    cr_assert_float_eq(h.normal.z, 0.0, 1e-15);
    cr_assert_null(h.material);
    cr_assert(h.frontFace);
}

Test(hitrecord, front_face_false_stored) {
    HitRecord h{1.0, {0, 0, 0}, {0, -1, 0}, nullptr, false};
    cr_assert_not(h.frontFace);
    cr_assert_float_eq(h.normal.y, -1.0, 1e-15);
}

Test(hitrecord, t_value_preserved) {
    HitRecord h{42.5, {0, 0, 0}, {1, 0, 0}, nullptr, true};
    cr_assert_float_eq(h.t, 42.5, 1e-15);
}

Test(hitrecord, null_material_allowed) {
    HitRecord h{1.0, {0, 0, 0}, {0, 1, 0}, nullptr, true};
    cr_assert_null(h.material);
}
