/*
 * Project: Raytracer
 * File name: Renderer.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Renderer implementation: closest-hit traversal, shadow ray casting, Lambert shading, and recursive material scattering up to a fixed depth.
 */

#include <iostream>
#include <limits>
#include "Renderer.hpp"
#include "IMaterial.hpp"
#include "ScatterResult.hpp"

constexpr int MAX_DEPTH = 50;

std::optional<HitRecord> Renderer::closestHit(const Ray &ray, const Scene &scene) {
    std::optional<HitRecord> closest;
    double tMax = std::numeric_limits<double>::infinity();

    for (const auto &primitive : scene.primitives()) {
        auto hit = primitive->intersect(ray);
        if (hit && hit->t > epsilon && hit->t < tMax) {
            tMax = hit->t;
            closest = hit;
        }
    }
    return closest;
}

void Renderer::render(const SceneContext &context) {
    int width = context.camera.getWidth();
    int height = context.camera.getHeight();

    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            Ray ray = context.camera.generateRay(x, y);
            Color pixelColor = traceRay(ray, context.scene, MAX_DEPTH);
            std::cout << toPPMByte(pixelColor.x) << ' '
                      << toPPMByte(pixelColor.y) << ' '
                      << toPPMByte(pixelColor.z) << '\n';
        }
    }
}

Color Renderer::traceRay(const Ray &ray, const Scene &scene, int depth) const {
    auto hit = closestHit(ray, scene);
    if (!hit)
        return Color{0, 0, 0};

    ScatterResult scattered = hit->material->scatter(ray, *hit);

    Color lightContrib{0, 0, 0};
    for (auto &light : scene.lights()) {
        LightSample sample = light->getSample(hit->point, hit->normal);

        if (sample.direction.x == 0 && sample.direction.y == 0 && sample.direction.z == 0) {
            lightContrib += sample.color;
            continue;
        }

        Ray shadowRay{hit->point, sample.direction};
        auto blocker = closestHit(shadowRay, scene);
        if (blocker && blocker->t < sample.distance)
            continue;

        double diffuse = std::max(0.0, dot(hit->normal, sample.direction));
        lightContrib += sample.color * diffuse;
    }

    if (depth > 0 && scattered.scatteredRay) {
        Color indirect = traceRay(*scattered.scatteredRay, scene, depth - 1);
        return scattered.attenuation * (lightContrib + indirect);
    }
    return scattered.attenuation * lightContrib;
}
