/*
 * Project: Raytracer
 * File name: test_scene.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for Scene - primitive and light management.
 */

#include <criterion/criterion.h>
#include "Scene.hpp"
#include "IPrimitive.hpp"
#include "ILight.hpp"

class MockPrimitive : public IPrimitive {
public:
    std::optional<HitRecord> intersect(const Ray &) const override { return std::nullopt; }
    void configure(const std::unordered_map<std::string, double> &, std::shared_ptr<IMaterial>) override {}
};

class MockLight : public ILight {
public:
    LightSample getSample(const Vec3 &, const Vec3 &) const override {
        return {{0, 0, 0}, {1, 1, 1}, 1.0};
    }
};

static PrimitivePtr makeMockPrimitive() {
    return PrimitivePtr(new MockPrimitive(), [](IPrimitive *p) { delete p; });
}

Test(scene, empty_scene_has_no_primitives) {
    Scene s;
    cr_assert(s.primitives().empty());
}

Test(scene, empty_scene_has_no_lights) {
    Scene s;
    cr_assert(s.lights().empty());
}

Test(scene, addPrimitive_increases_primitives_count) {
    Scene s;
    s.addPrimitive(makeMockPrimitive());
    cr_assert_eq(s.primitives().size(), 1u);
}

Test(scene, addLight_increases_lights_count) {
    Scene s;
    s.addLight(std::make_unique<MockLight>());
    cr_assert_eq(s.lights().size(), 1u);
}

Test(scene, add_multiple_primitives) {
    Scene s;
    s.addPrimitive(makeMockPrimitive());
    s.addPrimitive(makeMockPrimitive());
    s.addPrimitive(makeMockPrimitive());
    cr_assert_eq(s.primitives().size(), 3u);
}

Test(scene, add_multiple_lights) {
    Scene s;
    s.addLight(std::make_unique<MockLight>());
    s.addLight(std::make_unique<MockLight>());
    s.addLight(std::make_unique<MockLight>());
    cr_assert_eq(s.lights().size(), 3u);
}

Test(scene, primitives_contains_added_primitive) {
    Scene s;
    auto p = makeMockPrimitive();
    IPrimitive* raw = p.get();
    s.addPrimitive(std::move(p));
    cr_assert_eq(s.primitives()[0].get(), raw);
}

Test(scene, lights_contains_added_light) {
    Scene s;
    auto l = std::make_unique<MockLight>();
    ILight* raw = l.get();
    s.addLight(std::move(l));
    cr_assert_eq(s.lights()[0].get(), raw);
}

Test(scene, move_constructor_transfers_primitives_and_lights) {
    Scene s1;
    s1.addPrimitive(makeMockPrimitive());
    s1.addLight(std::make_unique<MockLight>());
    Scene s2 = std::move(s1);
    cr_assert_eq(s2.primitives().size(), 1u);
    cr_assert_eq(s2.lights().size(), 1u);
    cr_assert(s1.primitives().empty());
    cr_assert(s1.lights().empty());
}

Test(scene, move_assignment_transfers_primitives_and_lights) {
    Scene s1;
    s1.addPrimitive(makeMockPrimitive());
    s1.addLight(std::make_unique<MockLight>());
    Scene s2;
    s2 = std::move(s1);
    cr_assert_eq(s2.primitives().size(), 1u);
    cr_assert_eq(s2.lights().size(), 1u);
    cr_assert(s1.primitives().empty());
    cr_assert(s1.lights().empty());
}

Test(scene, primitive_bounding_box_defaults_to_infinite) {
    auto p = makeMockPrimitive();
    AABB box = p->boundingBox();
    cr_assert(box.isInfinite());
}

Test(scene, primitive_setFilePath_default_does_nothing) {
    auto p = makeMockPrimitive();
    cr_assert_no_throw(p->setFilePath("dummy"));
}
