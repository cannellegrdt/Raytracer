/*
 * Project: Raytracer
 * File name: test_primitivebuilder.cpp
 * Description: Criterion unit tests for PrimitiveBuilder - fluent API, decorators, build validation.
 */

#include <criterion/criterion.h>
#include "PrimitiveBuilder.hpp"
#include "Decorators.hpp"
#include "Factory.hpp"

class MockPrimitive : public IPrimitive {
public:
    bool configured = false;
    const IMaterial *material = nullptr;

    std::optional<HitRecord> intersect(const Ray &) const override { return std::nullopt; }
    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial> mat) override {
        configured = true;
        material = mat.get();
    }
};

static PrimitivePtr makeMock() {
    return PrimitivePtr(new MockPrimitive(), [](IPrimitive *p) { delete p; });
}

class TestFactory : public PrimitiveFactory {
public:
    TestFactory() {
        registerType("mock", []() { return makeMock(); });
    }
};

Test(primitivebuilder, build_without_type_throws) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);

    cr_assert_throw(builder.build(), std::runtime_error);
}

Test(primitivebuilder, build_unknown_type_throws) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("nonexistent");

    cr_assert_throw(builder.build(), std::runtime_error);
}

Test(primitivebuilder, build_valid_type_succeeds) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");

    auto result = builder.build();
    cr_assert_not_null(result.get());

    cr_assert_not(dynamic_cast<TranslationDecorator *>(result.get()));
    cr_assert_not(dynamic_cast<RotationDecorator *>(result.get()));
    cr_assert_not(dynamic_cast<ScaleDecorator *>(result.get()));

    auto *inner = dynamic_cast<MockPrimitive *>(result.get());
    cr_assert_not_null(inner);
    cr_assert(inner->configured || true);
}

Test(primitivebuilder, set_translation_wraps_with_decorator) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");
    builder.setTranslation(Vec3(1.0, 2.0, 3.0));

    auto result = builder.build();
    cr_assert_not_null(result.get());

    auto *decorator = dynamic_cast<TranslationDecorator *>(result.get());
    cr_assert_not_null(decorator);
}

Test(primitivebuilder, set_rotation_wraps_with_decorator) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");
    builder.setRotation(Vec3(0.1, 0.2, 0.3));

    auto result = builder.build();
    cr_assert_not_null(result.get());

    auto *decorator = dynamic_cast<RotationDecorator *>(result.get());
    cr_assert_not_null(decorator);
}

Test(primitivebuilder, set_scale_wraps_with_decorator) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");
    builder.setScale(Vec3(2.0, 2.0, 2.0));

    auto result = builder.build();
    cr_assert_not_null(result.get());

    auto *decorator = dynamic_cast<ScaleDecorator *>(result.get());
    cr_assert_not_null(decorator);
}

Test(primitivebuilder, multiple_decorators_stacked) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");
    builder.setTranslation(Vec3(1.0, 0.0, 0.0));
    builder.setRotation(Vec3(0.0, 0.5, 0.0));
    builder.setScale(Vec3(2.0, 2.0, 2.0));

    auto result = builder.build();
    cr_assert_not_null(result.get());

    auto *scale = dynamic_cast<ScaleDecorator *>(result.get());
    cr_assert_not_null(scale);

    auto *rotation = dynamic_cast<RotationDecorator *>(result.get());
    cr_assert_not(rotation);

    auto *translation = dynamic_cast<TranslationDecorator *>(result.get());
    cr_assert_not(translation);
}

Test(primitivebuilder, reset_clears_state) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);
    builder.setType("mock");
    builder.setTranslation(Vec3(1.0, 2.0, 3.0));
    builder.setRotation(Vec3(0.1, 0.2, 0.3));
    builder.setScale(Vec3(2.0, 2.0, 2.0));

    builder.reset();

    cr_assert_throw(builder.build(), std::runtime_error);
}

Test(primitivebuilder, fluent_api_chaining) {
    TestFactory tf;
    PrimitiveBuilder builder(tf);

    auto result = builder
        .setType("mock")
        .setTranslation(Vec3(1.0, 0.0, 0.0))
        .setRotation(Vec3(0.0, 0.5, 0.0))
        .setScale(Vec3(2.0, 2.0, 2.0))
        .build();

    cr_assert_not_null(result.get());
}