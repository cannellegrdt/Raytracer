/*
 * Project: Raytracer
 * File name: Renderer.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Renderer implementation: closest-hit traversal, shadow ray casting, Lambert shading, and recursive material scattering up to a fixed depth.
 */

#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>
#include "Renderer.hpp"
#include "IMaterial.hpp"
#include "ScatterResult.hpp"
#include "Common.hpp"

constexpr int MAX_DEPTH = 10;

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

static double diffColor(const Color &c1, const Color &c2) {
    return std::max({std::abs(c1.x - c2.x), std::abs(c1.y - c2.y), std::abs(c1.z - c2.z)});
}

void Renderer::render(const SceneContext &context, const std::string &outputPath) {
    if (!context.camera)
        throw std::runtime_error("No camera in scene context");
    const Camera &cam = *context.camera;
    int width = cam.getWidth();
    int height = cam.getHeight();

    int samples = (context.antialiasing) ? context.antialiasing->samples : 1;
    std::string aaType = (context.antialiasing) ? context.antialiasing->type : "uniform";
    double threshold = (context.antialiasing) ? context.antialiasing->threshold : 0.0;

    std::vector<Color> pixelBuffer(width * height);

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            Ray ray = cam.generateRay(static_cast<double>(x), static_cast<double>(y));
            pixelBuffer[y * width + x] = traceRay(ray, context.scene, MAX_DEPTH);
        }
    }

    if (aaType == "adaptive" && samples > 1) {
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                const Color &currentColor = pixelBuffer[y * width + x];
                double maxDiff = 0.0;

                if (x > 0) {
                    const Color &left = pixelBuffer[y * width + (x-1)];
                    maxDiff = std::max(maxDiff, diffColor(currentColor, left));
                }
                if (y > 0) {
                    const Color &up = pixelBuffer[(y-1) * width + x];
                    maxDiff = std::max(maxDiff, diffColor(currentColor, up));
                }
                if (x < width-1) {
                    const Color &right = pixelBuffer[y * width + (x+1)];
                    maxDiff = std::max(maxDiff, diffColor(currentColor, right));
                }
                if (y < height-1) {
                    const Color &down = pixelBuffer[(y+1) * width + x];
                    maxDiff = std::max(maxDiff, diffColor(currentColor, down));
                }

                if (maxDiff > threshold) {
                    Color refinedColor{0, 0, 0};
                    for (int j=0; j<samples; j++) {
                        for (int i=0; i<samples; i++) {
                            double u = static_cast<double>(x) + (static_cast<double>(i) + 0.5) / samples;
                            double v = static_cast<double>(y) + (static_cast<double>(j) + 0.5) / samples;
                            Ray ray = cam.generateRay(u, v);
                            refinedColor = refinedColor + traceRay(ray, context.scene, MAX_DEPTH);
                        }
                    }
                    pixelBuffer[y * width + x] = refinedColor * (1.0 / (samples * samples));
                }
            }
        }
    } else if (samples > 1) {
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                Color pixelColor{0, 0, 0};
                for (int j=0; j<samples; j++) {
                    for (int i=0; i<samples; i++) {
                        double u = static_cast<double>(x) + (static_cast<double>(i) + 0.5) / samples;
                        double v = static_cast<double>(y) + (static_cast<double>(j) + 0.5) / samples;
                        Ray ray = cam.generateRay(u, v);
                        pixelColor = pixelColor + traceRay(ray, context.scene, MAX_DEPTH);
                    }
                }
                pixelBuffer[y * width + x] = pixelColor * (1.0 / (samples * samples));
            }
        }
    }

    std::ofstream outFile(outputPath);
    if (!outFile)
        throw std::runtime_error("Error: cannot open output file '" + outputPath + "'");

    outFile << "P3\n" << width << ' ' << height << "\n255\n";

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            const Color &pixelColor = pixelBuffer[y * width + x];
            outFile << toPPMByte(pixelColor.x) << ' '
                    << toPPMByte(pixelColor.y) << ' '
                    << toPPMByte(pixelColor.z) << '\n';
        }
    }
    outFile.close();
    if (outFile.fail())
        throw std::runtime_error("Write error on output file: " + outputPath);
}

Color Renderer::traceRay(const Ray &ray, const Scene &scene, int depth) const {
    auto hit = closestHit(ray, scene);
    if (!hit)
        return Color{0, 0, 0};

    ScatterResult scattered = hit->material->scatter(ray, *hit);

    Color lightContrib{0, 0, 0};
    for (const auto &light : scene.lights()) {
        LightSample sample = light->getSample(hit->point, hit->normal);

        if (sample.isAmbient) {
            lightContrib += sample.color;
            continue;
        }

        Ray shadowRay{hit->point + RayBias * hit->normal, sample.direction};
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
