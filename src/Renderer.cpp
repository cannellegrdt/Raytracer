/*
 * Project: Raytracer
 * File name: Renderer.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Renderer implementation: closest-hit traversal, shadow ray casting, Lambert shading, and recursive material scattering up to a fixed depth.
 */

#include <iostream>
#include <fstream>
#include <limits>
#include <random>
#include <cmath>
#include "Renderer.hpp"
#include "IMaterial.hpp"
#include "ScatterResult.hpp"
#include "Common.hpp"

constexpr int MAX_DEPTH = 10;

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<double> dist(0.0, 1.0);

static Vec3 randomInUnitSphere() {
    while (true) {
        Vec3 p{dist(gen) * 2 - 1, dist(gen) * 2 - 1, dist(gen) * 2 - 1};
        if (dot(p, p) < 1.0) return p;
    }
}

static Vec3 randomInHemisphere(const Vec3 &normal) {
    Vec3 inUnitSphere = randomInUnitSphere();
    if (dot(inUnitSphere, normal) > 0.0)
        return inUnitSphere;
    return -inUnitSphere;
}

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
        std::vector<bool> needRefine(width * height, false);
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

                needRefine[y * width + x] = (maxDiff > threshold);
            }
        }

        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                if (!needRefine[y * width + x])
                    continue;

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
        return scene.backgroundColor();

    ScatterResult scattered = hit->material->scatter(ray, *hit);

    Color lightDiffuse{0, 0, 0};
    Color lightSpecular{0, 0, 0};
    for (const auto &light : scene.lights()) {
        LightSample sample = light->getSample(hit->point, hit->normal);

        if (sample.isAmbient) {
            int nbAORays = 16;
            int unoccluded = 0;

            for (int i=0; i<nbAORays; i++) {
                Vec3 AODir = randomInHemisphere(hit->normal);
                Ray AORay{hit->point + RayBias * hit->normal, AODir};
                auto AOHit = closestHit(AORay, scene);
                if (!AOHit || AOHit->t > sample.maxDistance)
                    unoccluded++;
            }

            double AOFactor = static_cast<double>(unoccluded) / nbAORays;
            lightDiffuse += sample.color * AOFactor;
            continue;
        }

        Ray shadowRay{hit->point + RayBias * hit->normal, sample.direction};
        auto blocker = closestHit(shadowRay, scene);
        if (blocker && blocker->t < sample.distance)
            continue;

        double diffuse = std::max(0.0, dot(hit->normal, sample.direction));
        lightDiffuse += sample.color * diffuse;

        auto specularParams = hit->material->getSpecular();
        if (specularParams) {
            Vec3 refl = reflect(-sample.direction, hit->normal);
            Vec3 normalDir = normalize(-ray.direction);
            double specAngle = std::max(0.0, dot(refl, normalDir));
            double specular = std::pow(specAngle, specularParams->shininess);
            lightSpecular += specularParams->ks * sample.color * specular;
        }
    }

    if (depth > 0 && scattered.scatteredRay) {
        Color indirect = traceRay(*scattered.scatteredRay, scene, depth - 1);
        return scattered.attenuation * indirect;
    }
    return scattered.attenuation * lightDiffuse + lightSpecular;
}
