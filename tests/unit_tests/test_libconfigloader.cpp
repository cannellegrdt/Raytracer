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
