/*
 * Project: Raytracer
 * File name: test_renderer.cpp
 * Description: Criterion unit tests for Renderer - ray tracing, lighting, and scattering.
 */

#include <criterion/criterion.h>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "Vec3.hpp"
#include "HitRecord.hpp"
#include "Color.hpp"
#include "ScatterResult.hpp"
#include "LightSample.hpp"
#include "IMaterial.hpp"
#include "IPrimitive.hpp"
#include "ILight.hpp"
#include "Type.hpp"

#ifdef UNIT_TEST
class RendererTestAccessor {
public:
    static Color traceRay(const Renderer &r, const Ray &ray, const Scene &scene, int depth) {
        return r.traceRay(ray, scene, depth);
    }
    static std::optional<HitRecord> closestHit(const Ray &ray, const Scene &scene) {
        return Renderer::closestHit(ray, scene);
    }
};
#endif

class MockMaterial : public IMaterial {
public:
    Vec3 attenuation;
    bool scatterEnabled;
    Ray scatteredRay;

    MockMaterial(Vec3 att, bool scatter = true, Ray scattered = Ray{Vec3{0,0,0}, Vec3{0,0,0}})
        : attenuation(att), scatterEnabled(scatter), scatteredRay(scattered) {}

    ScatterResult scatter(const Ray &, const HitRecord &) const override {
        if (scatterEnabled) {
            return ScatterResult{attenuation, scatteredRay};
        }
        return ScatterResult{attenuation, std::nullopt};
    }
};

class MockLight : public ILight {
public:
    Vec3 dir;
    Vec3 col;
    double dist;

    MockLight(const Vec3 &d, const Vec3 &c, double dval) : dir(d), col(c), dist(dval) {}

    LightSample getSample(const Vec3 &, const Vec3 &) const override {
        return LightSample{dir, col, dist};
    }
};

class ConditionalPrimitive : public IPrimitive {
    std::function<bool(const Ray &)> condition;
    double tHit;
    Vec3 normalVal;
    const IMaterial *mat;

public:
    ConditionalPrimitive(std::function<bool(const Ray &)> cond, double t, const Vec3 &norm, const IMaterial *m = nullptr)
        : condition(std::move(cond)), tHit(t), normalVal(norm), mat(m) {}

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        if (!condition(ray)) return std::nullopt;
        HitRecord rec;
        rec.t = tHit;
        rec.point = ray.at(tHit);
        rec.normal = normalVal;
        rec.material = mat;
        rec.frontFace = true;
        return rec;
    }

    void configure(const std::unordered_map<std::string, double> &, const IMaterial *m) override {
        mat = m;
    }
};

static void addConditionalPrimitive(Scene &scene,
                                    std::function<bool(const Ray &)> condition,
                                    double t,
                                    const Vec3 &normal,
                                    const IMaterial *mat = nullptr) {
    PrimitivePtr ptr(new ConditionalPrimitive(std::move(condition), t, normal, mat),
                     [](IPrimitive *p) { delete p; });
    scene.addPrimitive(std::move(ptr));
}

Test(traceRay, returns_black_when_no_hit) {
    Renderer renderer;
    Scene scene;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 1);
    cr_assert_float_eq(result.x, 0.0, 1e-10);
    cr_assert_float_eq(result.y, 0.0, 1e-10);
    cr_assert_float_eq(result.z, 0.0, 1e-10);
}

Test(traceRay, computes_direct_lighting) {
    MockMaterial mat(Vec3(1, 1, 1), false);
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(0, 0, 1), &mat);

    scene.addLight(std::make_unique<MockLight>(Vec3(0, 0, 1), Vec3(1, 1, 1), 10.0));

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 0);

    cr_assert_float_eq(result.x, 1.0, 1e-10);
    cr_assert_float_eq(result.y, 1.0, 1e-10);
    cr_assert_float_eq(result.z, 1.0, 1e-10);
}

Test(traceRay, respects_max_depth_no_recursion) {
    MockMaterial mat(Vec3(1, 1, 1), true);
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(0, 0, 1), &mat);

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 0);

    cr_assert_float_eq(result.x, 0.0, 1e-10);
    cr_assert_float_eq(result.y, 0.0, 1e-10);
    cr_assert_float_eq(result.z, 0.0, 1e-10);
}

Test(traceRay, blocks_light_when_shadow_ray_hits) {
    MockMaterial mat(Vec3(1, 1, 1), false);
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(0, 0, 1), &mat);

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == -5; },
        3.0, Vec3(0, 0, 1), nullptr);

    scene.addLight(std::make_unique<MockLight>(Vec3(1, 0, 0), Vec3(1, 1, 1), 10.0));

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 0);

    cr_assert_float_eq(result.x, 0.0, 1e-10);
    cr_assert_float_eq(result.y, 0.0, 1e-10);
    cr_assert_float_eq(result.z, 0.0, 1e-10);
}

Test(traceRay, allows_light_when_no_blocker) {
    MockMaterial mat(Vec3(1, 1, 1), false);
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(1, 0, 0), &mat);

    scene.addLight(std::make_unique<MockLight>(Vec3(1, 0, 0), Vec3(0.5, 0.5, 0.5), 10.0));

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 0);

    cr_assert_float_eq(result.x, 0.5, 1e-10);
    cr_assert_float_eq(result.y, 0.5, 1e-10);
    cr_assert_float_eq(result.z, 0.5, 1e-10);
}

Test(traceRay, no_contribution_when_normal_perpendicular_to_light) {
    MockMaterial mat(Vec3(1, 1, 1), false);
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(0, 0, 1), &mat);

    scene.addLight(std::make_unique<MockLight>(Vec3(1, 0, 0), Vec3(1, 1, 1), 10.0));

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 0);

    cr_assert_float_eq(result.x, 0.0, 1e-10);
    cr_assert_float_eq(result.y, 0.0, 1e-10);
    cr_assert_float_eq(result.z, 0.0, 1e-10);
}

Test(traceRay, handles_scattered_ray_recursion) {
    MockMaterial mat(Vec3(0.5, 0.5, 0.5), true,
                     Ray(Vec3(0, 0, -5), Vec3(0, 0, -1)));
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &r) { return r.origin.x == 0 && r.origin.y == 0 && r.origin.z == 0; },
        5.0, Vec3(0, 0, 1), &mat);

    scene.addLight(std::make_unique<MockLight>(Vec3(0, 0, 1), Vec3(1, 1, 1), 10.0));

    Renderer renderer;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    Color result = RendererTestAccessor::traceRay(renderer, ray, scene, 1);

    cr_assert_float_eq(result.x, 0.5, 1e-10);
    cr_assert_float_eq(result.y, 0.5, 1e-10);
    cr_assert_float_eq(result.z, 0.5, 1e-10);
}

Test(closestHit, returns_nullopt_on_no_intersection) {
    Scene scene;
    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    auto hit = RendererTestAccessor::closestHit(ray, scene);
    cr_assert_not(hit.has_value());
}

Test(closestHit, finds_closest_among_multiple) {
    Scene scene;

    addConditionalPrimitive(scene,
        [](const Ray &) { return true; },
        3.0, Vec3(0, 0, 1), nullptr);

    addConditionalPrimitive(scene,
        [](const Ray &) { return true; },
        7.0, Vec3(0, 0, 1), nullptr);

    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    auto hit = RendererTestAccessor::closestHit(ray, scene);

    cr_assert(hit.has_value(), "closestHit should return a hit");
    cr_assert_float_eq(hit->t, 3.0, 1e-10);
}

Test(closestHit, respects_epsilon_threshold) {
    Scene scene;
    addConditionalPrimitive(scene,
        [](const Ray &) { return true; },
        1e-13, Vec3(0, 0, 1), nullptr);

    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1));
    auto hit = RendererTestAccessor::closestHit(ray, scene);

    cr_assert_not(hit.has_value());
}

Test(closestHit, ignores_hits_behind_ray_origin) {
    Scene scene;
    addConditionalPrimitive(scene,
        [](const Ray &) { return true; },
        -5.0, Vec3(0, 0, -1), nullptr);

    Ray ray(Vec3(0, 0, 0), Vec3(0, 0, 1));
    auto hit = RendererTestAccessor::closestHit(ray, scene);

    cr_assert_not(hit.has_value(), "Negative t should be rejected");
}

Test(closestHit, picks_one_when_tie) {
    Scene scene;
    addConditionalPrimitive(scene, [](const Ray&){return true;}, 5.0, Vec3(0,0,1), nullptr);
    addConditionalPrimitive(scene, [](const Ray&){return true;}, 5.0, Vec3(0,0,1), nullptr);
    Ray ray(Vec3(0,0,0), Vec3(0,0,-1));
    auto hit = RendererTestAccessor::closestHit(ray, scene);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 5.0, 1e-10);
}
