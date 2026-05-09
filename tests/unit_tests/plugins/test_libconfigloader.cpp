/*
 * Project: Raytracer
 * File name: test_libconfigloader.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for LibconfigLoader - error handling tests.
 */

#include <criterion/criterion.h>
#include <string>
#include "LibconfigLoader.hpp"
#include "Factory.hpp"
#include "Type.hpp"

struct MockPrimForLoader : IPrimitive {
    std::optional<HitRecord> intersect(const Ray &) const override { return std::nullopt; }
    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}
};

namespace {

std::string fixturePath(const char *name) {
    return std::string("tests/unit_tests/fixtures/") + name;
}

}

Test(libconfigloader, load_nonexistent_file_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;

    cr_assert_throw(loader.load(fixturePath("nonexistent.cfg"), factory), std::runtime_error);
}

Test(libconfigloader, load_invalid_material_type_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;

    try {
        loader.load(fixturePath("invalid_material.cfg"), factory);
        cr_assert_fail("Expected exception was not thrown");
    } catch (const std::runtime_error &e) {
        cr_assert(true);
    }
}

Test(libconfigloader, nonexistent_file_error_contains_filename) {
    LibconfigLoader loader;
    PrimitiveFactory factory;

    try {
        loader.load(fixturePath("nonexistent.cfg"), factory);
        cr_assert_fail("Expected exception was not thrown");
    } catch (const std::runtime_error &e) {
        cr_assert_str_neq(e.what(), "");
    }
}

Test(libconfigloader, load_with_transformation_matrix_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_transformation_matrix.cfg"), factory));
}

Test(libconfigloader, load_with_groups_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_groups.cfg"), factory));
}

Test(libconfigloader, load_with_nested_groups_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_nested_groups.cfg"), factory));
}

Test(libconfigloader, load_with_group_translation_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_group_translation.cfg"), factory));
}

Test(libconfigloader, load_textured_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_textured_material.cfg"), factory));
}

Test(libconfigloader, load_chessboard_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_chessboard_material.cfg"), factory));
}

Test(libconfigloader, load_marble_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_marble_material.cfg"), factory));
}

Test(libconfigloader, load_normalmap_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_normalmap_material.cfg"), factory));
}

Test(libconfigloader, load_transparency_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_transparency_material.cfg"), factory));
}

Test(libconfigloader, load_phong_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_phong_material.cfg"), factory));
}

Test(libconfigloader, load_refraction_material_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_refraction_material.cfg"), factory));
}

Test(libconfigloader, load_invalid_ior_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_throw(loader.load(fixturePath("invalid_ior.cfg"), factory), std::runtime_error);
}

Test(libconfigloader, load_torus_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("torus", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_torus.cfg"), factory));
}

Test(libconfigloader, load_cube_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("cube", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_cube.cfg"), factory));
}

Test(libconfigloader, load_tanglecube_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("tanglecube", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_tanglecube.cfg"), factory));
}

Test(libconfigloader, load_triangle_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("triangle", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_triangle.cfg"), factory));
}

Test(libconfigloader, load_mandelbulb_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("mandelbulb", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_mandelbulb.cfg"), factory));
}

Test(libconfigloader, load_mobius_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("mobius", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_mobius.cfg"), factory));
}

Test(libconfigloader, load_with_scale_transform_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_scale_transform.cfg"), factory));
}

Test(libconfigloader, load_with_shear_transform_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_shear_transform.cfg"), factory));
}

Test(libconfigloader, load_with_rotation_transform_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_rotation_transform.cfg"), factory));
}

Test(libconfigloader, load_with_translation_transform_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_translation_transform.cfg"), factory));
}

Test(libconfigloader, load_with_matrix_transform_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_matrix_transform.cfg"), factory));
}

Test(libconfigloader, load_with_uniform_antialiasing_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_uniform_antialiasing.cfg"), factory));
}

Test(libconfigloader, load_with_adaptive_antialiasing_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_adaptive_antialiasing.cfg"), factory));
}

Test(libconfigloader, load_with_nbAORays_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_renderer_aorays.cfg"), factory));
}

Test(libconfigloader, load_with_point_light_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_point_light.cfg"), factory));
}

Test(libconfigloader, load_with_ambient_maxdist_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_ambient_maxdist.cfg"), factory));
}

Test(libconfigloader, load_with_background_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_background.cfg"), factory));
}

Test(libconfigloader, load_with_imports_succeeds) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_no_throw(loader.load(fixturePath("valid_imports.cfg"), factory));
}

Test(libconfigloader, load_circular_import_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_throw(loader.load(fixturePath("circular_a.cfg"), factory), std::runtime_error);
}

Test(libconfigloader, load_parse_error_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_throw(loader.load(fixturePath("parse_error.cfg"), factory), std::runtime_error);
}

Test(libconfigloader, load_missing_camera_throws_runtime_error) {
    LibconfigLoader loader;
    PrimitiveFactory factory;
    factory.registerType("sphere", []() {
        return PrimitivePtr(new MockPrimForLoader(), [](IPrimitive *p) { delete p; });
    });

    cr_assert_throw(loader.load(fixturePath("missing_camera.cfg"), factory), std::runtime_error);
}
