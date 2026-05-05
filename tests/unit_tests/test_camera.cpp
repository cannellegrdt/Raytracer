/*
 * Project: Raytracer
 * File name: test_camera.cpp
 * Description: Criterion unit tests for Camera - construction, ray generation.
 */

#include <criterion/criterion.h>
#include <cmath>
#include "Camera.hpp"
#include "Vec3.hpp"

Test(camera, construction_stores_parameters) {
    Vec3 position(1.0, 2.0, 3.0);
    Vec3 rotation(10.0, 20.0, 30.0);
    double fov = 90.0;
    int width = 800;
    int height = 600;

    Camera cam(position, rotation, fov, width, height);
    Ray ray = cam.generateRay(0.0, 0.0);

    cr_assert_float_eq(ray.origin.x, position.x, 1e-15);
    cr_assert_float_eq(ray.origin.y, position.y, 1e-15);
    cr_assert_float_eq(ray.origin.z, position.z, 1e-15);
}

Test(camera, generate_ray_origin_is_camera_position) {
    Vec3 position(5.0, -3.0, 2.0);
    Camera cam(position, Vec3(0, 0, 0), 60.0, 640, 480);
    Ray ray = cam.generateRay(320.0, 240.0);

    cr_assert_float_eq(ray.origin.x, position.x, 1e-15);
    cr_assert_float_eq(ray.origin.y, position.y, 1e-15);
    cr_assert_float_eq(ray.origin.z, position.z, 1e-15);
}

Test(camera, generate_ray_direction_is_normalized) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 1, 1);
    Ray ray = cam.generateRay(0.0, 0.0);

    cr_assert_float_eq(length(ray.direction), 1.0, 1e-10);
}

Test(camera, center_pixel_points_near_forward) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 1, 1);
    Ray ray = cam.generateRay(0.0, 0.0);

    cr_assert_float_eq(ray.direction.x, 0.0, 1e-10);
    cr_assert_float_eq(ray.direction.y, 0.0, 1e-10);
    cr_assert_float_eq(ray.direction.z, 1.0, 1e-10);
}

Test(camera, top_left_pixel_points_left_upward) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 100, 100);
    Ray ray = cam.generateRay(0.0, 0.0);

    cr_assert(ray.direction.x < 0.0, "top-left should point left");
    cr_assert(ray.direction.y > 0.0, "top-left should point up");
    cr_assert(ray.direction.z > 0.0, "top-left should point forward");
}

Test(camera, bottom_right_pixel_points_right_downward) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 100, 100);
    Ray ray = cam.generateRay(99.0, 99.0);

    cr_assert(ray.direction.x > 0.0, "bottom-right should point right");
    cr_assert(ray.direction.y < 0.0, "bottom-right should point down");
    cr_assert(ray.direction.z > 0.0, "bottom-right should point forward");
}

Test(camera, aspect_ratio_affects_horizontal_spread) {
    Camera camWide(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 200, 100);
    Camera camTall(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 100, 200);

    Ray rayWide = camWide.generateRay(199.0, 99.0);
    Ray rayTall = camTall.generateRay(99.0, 199.0);

    cr_assert(rayTall.direction.x != rayWide.direction.x, "wide vs tall aspect ratios produce different X");
}

Test(camera, different_fov_produces_different_directions) {
    Camera cam60(Vec3(0, 0, 0), Vec3(0, 0, 0), 60.0, 100, 100);
    Camera cam120(Vec3(0, 0, 0), Vec3(0, 0, 0), 120.0, 100, 100);

    Ray ray60 = cam60.generateRay(99.0, 99.0);
    Ray ray120 = cam120.generateRay(99.0, 99.0);

    cr_assert(ray60.direction.x != ray120.direction.x, "different FOVs should produce different X");
    cr_assert(ray60.direction.y != ray120.direction.y, "different FOVs should produce different Y");
}

Test(camera, zero_rotation_produces_forward_direction) {
    Camera cam(Vec3(10, 5, -3), Vec3(0, 0, 0), 60.0, 1, 1);
    Ray ray = cam.generateRay(0.0, 0.0);

    cr_assert_float_eq(ray.direction.x, 0.0, 1e-10);
    cr_assert_float_eq(ray.direction.y, 0.0, 1e-10);
    cr_assert_float_eq(ray.direction.z, 1.0, 1e-10);
}

Test(camera, getWidth_returns_constructed_width) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 800, 600);
    cr_assert_eq(cam.getWidth(), 800);
}

Test(camera, getHeight_returns_constructed_height) {
    Camera cam(Vec3(0, 0, 0), Vec3(0, 0, 0), 90.0, 800, 600);
    cr_assert_eq(cam.getHeight(), 600);
}