/*
 * Project: Raytracer
 * File name: test_mandelbulb.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the Mandelbulb plugin - configuration, bounding box, and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>
#include <memory>

#include <interfaces/IMaterial.hpp>

#define create mandelbulb_create_fn
#define destroy mandelbulb_destroy_fn
#include "../../src/plugins/mandelbulb.cpp"
#undef create
#undef destroy

static std::shared_ptr<IMaterial> dummyMaterial() {
    struct DummyMat : public IMaterial {
        ScatterResult scatter(const Ray &, const HitRecord &) const override {
            return {{1.0, 1.0, 1.0}, std::nullopt, std::nullopt};
        }
    };
    return std::shared_ptr<IMaterial>(new DummyMat());
}

Test(mandelbulb, plugin_create_returns_non_null) {
    IPrimitive *p = mandelbulb_create_fn();
    cr_assert_not_null(p);
    mandelbulb_destroy_fn(p);
}

Test(mandelbulb, plugin_destroy_deletes_object) {
    IPrimitive *p = mandelbulb_create_fn();
    cr_assert_no_throw(mandelbulb_destroy_fn(p));
}

Test(mandelbulb, configure_with_valid_params_no_throw) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    cr_assert_no_throw(bulb.configure(params, dummyMaterial()));
}

Test(mandelbulb, configure_sets_center_correctly) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 1.0}, {"y", 2.0}, {"z", 3.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    AABB box = bulb.boundingBox();
    cr_assert_float_eq(box.min.x, -1.0, 1e-10);
    cr_assert_float_eq(box.min.y, 0.0, 1e-10);
    cr_assert_float_eq(box.min.z, 1.0, 1e-10);
    cr_assert_float_eq(box.max.x, 3.0, 1e-10);
    cr_assert_float_eq(box.max.y, 4.0, 1e-10);
    cr_assert_float_eq(box.max.z, 5.0, 1e-10);
}

Test(mandelbulb, bounding_box_valid_after_default_configure) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    AABB box = bulb.boundingBox();
    cr_assert(box.min.x < box.max.x);
    cr_assert(box.min.y < box.max.y);
    cr_assert(box.min.z < box.max.z);
    cr_assert_float_eq(box.min.x, -2.0, 1e-10);
    cr_assert_float_eq(box.min.y, -2.0, 1e-10);
    cr_assert_float_eq(box.min.z, -2.0, 1e-10);
    cr_assert_float_eq(box.max.x, 2.0, 1e-10);
    cr_assert_float_eq(box.max.y, 2.0, 1e-10);
    cr_assert_float_eq(box.max.z, 2.0, 1e-10);
}

Test(mandelbulb, ray_misses_outside_bounding_box) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{10.0, 10.0, 10.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(mandelbulb, ray_hits_center_approaches_origin) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
}

Test(mandelbulb, hit_point_matches_ray_at_t) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-6);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-6);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-6);
}

Test(mandelbulb, normal_is_unit_length) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-6);
}

Test(mandelbulb, front_face_true_on_outside_hit) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(mandelbulb, uv_coordinates_in_valid_range) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(mandelbulb, tangent_and_bitangent_are_orthogonal_to_normal) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    double dot_tangent = dot(hit->tangent, hit->normal);
    double dot_bitangent = dot(hit->bitangent, hit->normal);
    cr_assert_float_eq(dot_tangent, 0.0, 1e-6);
    cr_assert_float_eq(dot_bitangent, 0.0, 1e-6);
    double dot_t_b = dot(hit->tangent, hit->bitangent);
    cr_assert_float_eq(dot_t_b, 0.0, 1e-6);
}

Test(mandelbulb, scaled_mandelbulb_hits_correctly) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 0.0}, {"y", 0.0}, {"z", 0.0},
        {"s", 2.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    Ray ray{{0.0, 0.0, 6.0}, {0.0, 0.0, -1.0}};
    auto hit = bulb.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->t > 2.0);
}

Test(mandelbulb, translated_mandelbulb_bounding_box_correct) {
    Mandelbulb bulb;
    std::unordered_map<std::string, double> params = {
        {"x", 5.0}, {"y", 5.0}, {"z", 5.0},
        {"s", 1.0}, {"power", 8.0}, {"iters", 20.0},
        {"bailout", 2.0}
    };
    bulb.configure(params, dummyMaterial());
    AABB box = bulb.boundingBox();
    cr_assert_float_eq(box.min.x, 3.0, 1e-10);
    cr_assert_float_eq(box.min.y, 3.0, 1e-10);
    cr_assert_float_eq(box.min.z, 3.0, 1e-10);
    cr_assert_float_eq(box.max.x, 7.0, 1e-10);
    cr_assert_float_eq(box.max.y, 7.0, 1e-10);
    cr_assert_float_eq(box.max.z, 7.0, 1e-10);
}