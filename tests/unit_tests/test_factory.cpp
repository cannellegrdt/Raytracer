/*
 * Project: Raytracer
 * File name: test_factory.cpp
 * Description: Criterion unit tests for Factory<T> - type registration, creation, and miss handling.
 */

#include <criterion/criterion.h>
#include "Factory.hpp"

class MockPrimitive : public IPrimitive {
public:
    int tag;
    explicit MockPrimitive(int tag = 0) : tag(tag) {}
    std::optional<HitRecord> intersect(const Ray &) const override { return std::nullopt; }
    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}
};

static PrimitivePtr makeMock(int tag = 0) {
    return PrimitivePtr(new MockPrimitive(tag), [](IPrimitive *p) { delete p; });
}

Test(factory, registered_key_creates_object) {
    PrimitiveFactory f;
    f.registerType("mock", []() { return makeMock(); });
    auto result = f.create("mock");
    cr_assert(result.has_value());
    cr_assert_not_null(result->get());
}

Test(factory, unregistered_key_returns_nullopt) {
    PrimitiveFactory f;
    auto result = f.create("nonexistent");
    cr_assert_not(result.has_value());
}

Test(factory, empty_factory_returns_nullopt) {
    PrimitiveFactory f;
    cr_assert_not(f.create("anything").has_value());
}

Test(factory, multiple_types_independent) {
    PrimitiveFactory f;
    f.registerType("a", []() { return makeMock(1); });
    f.registerType("b", []() { return makeMock(2); });
    auto ra = f.create("a");
    auto rb = f.create("b");
    cr_assert(ra.has_value());
    cr_assert(rb.has_value());
    cr_assert_not_null(ra->get());
    cr_assert_not_null(rb->get());
}

Test(factory, create_called_multiple_times_returns_new_objects) {
    PrimitiveFactory f;
    f.registerType("mock", []() { return makeMock(); });
    auto r1 = f.create("mock");
    auto r2 = f.create("mock");
    cr_assert(r1.has_value());
    cr_assert(r2.has_value());
    cr_assert(r1->get() != r2->get());
}

Test(factory, overwrite_registration_replaces_creator) {
    PrimitiveFactory f;
    f.registerType("mock", []() { return makeMock(1); });
    f.registerType("mock", []() { return makeMock(2); });
    auto result = f.create("mock");
    cr_assert(result.has_value());
    cr_assert_not_null(result->get());
    cr_assert_eq(static_cast<MockPrimitive *>(result->get())->tag, 2);
}

Test(factory, unregistered_key_does_not_affect_registered) {
    PrimitiveFactory f;
    f.registerType("real", []() { return makeMock(); });
    cr_assert_not(f.create("fake").has_value());
    cr_assert(f.create("real").has_value());
}
