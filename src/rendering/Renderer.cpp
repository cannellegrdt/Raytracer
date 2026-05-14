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
#include <vector>
#include <cstdint>
#include <algorithm>
#include <thread>
#include <atomic>
#include <omp.h>
#include <SFML/Graphics.hpp>
#include "Renderer.hpp"
#include "IMaterial.hpp"
#include "ScatterResult.hpp"
#include "Common.hpp"
#include "BVH.hpp"

constexpr int MAX_DEPTH = 10;
constexpr int TILE_SIZE = 16;

thread_local std::mt19937 gen(std::random_device{}());
thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);

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
    if (const BVH *bvh = scene.bvh())
        return bvh->intersect(ray, scene.primitives());

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

static bool shouldStopCheck(const std::atomic<bool> *shouldStop) {
    return shouldStop && shouldStop->load(std::memory_order_relaxed);
}

void Renderer::renderTiles(const SceneContext &context, std::vector<Color> &pixelBuffer,
    const RenderParams &params, const std::atomic<bool> *shouldStop) const {
    const Camera &cam = *context.camera;
    int width = params.width;
    int height = params.height;
    int samples = params.samples;
    const std::string &aaType = params.aaType;
    double threshold = params.threshold;
    int nbAORays = params.nbAORays;

    if (samples == 1 || aaType == "adaptive") {
        int tileHeight = (height + TILE_SIZE - 1) / TILE_SIZE;
        int tileWidth = (width + TILE_SIZE - 1) / TILE_SIZE;
        #pragma omp parallel for schedule(dynamic, 1)
        for (int ty=0; ty<tileHeight; ty++) {
            for (int tx=0; tx<tileWidth; tx++) {
                if (shouldStopCheck(shouldStop)) continue;
                int yStart = ty * TILE_SIZE;
                int xStart = tx * TILE_SIZE;
                int yEnd = std::min(yStart + TILE_SIZE, height);
                int xEnd = std::min(xStart + TILE_SIZE, width);
                for (int y=yStart; y<yEnd; y++) {
                    for (int x=xStart; x<xEnd; x++) {
                        Ray ray = cam.generateRay(static_cast<double>(x), static_cast<double>(y));
                        pixelBuffer[y * width + x] = traceRay(ray, context.scene, MAX_DEPTH, nbAORays);
                    }
                }
            }
        }
    }

    if (shouldStopCheck(shouldStop)) return;

    if (aaType == "adaptive" && samples > 1) {
        std::vector<bool> needRefine(width * height, false);
        #pragma omp parallel for schedule(dynamic, 4)
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

        if (shouldStopCheck(shouldStop)) return;

        int tileHeight = (height + TILE_SIZE - 1) / TILE_SIZE;
        int tileWidth = (width + TILE_SIZE - 1) / TILE_SIZE;
        #pragma omp parallel for schedule(dynamic, 1)
        for (int ty=0; ty<tileHeight; ty++) {
            for (int tx=0; tx<tileWidth; tx++) {
                if (shouldStopCheck(shouldStop)) continue;
                int yStart = ty * TILE_SIZE;
                int xStart = tx * TILE_SIZE;
                int yEnd = std::min(yStart + TILE_SIZE, height);
                int xEnd = std::min(xStart + TILE_SIZE, width);
                for (int y=yStart; y<yEnd; y++) {
                    for (int x=xStart; x<xEnd; x++) {
                        if (!needRefine[y * width + x])
                            continue;

                        Color refinedColor{0, 0, 0};
                        for (int j=0; j<samples; j++) {
                            for (int i=0; i<samples; i++) {
                                double u = static_cast<double>(x) + (static_cast<double>(i) + 0.5) / samples;
                                double v = static_cast<double>(y) + (static_cast<double>(j) + 0.5) / samples;
                                Ray ray = cam.generateRay(u, v);
                                refinedColor = refinedColor + traceRay(ray, context.scene, MAX_DEPTH, nbAORays);
                            }
                        }
                        pixelBuffer[y * width + x] = refinedColor * (1.0 / (samples * samples));
                    }
                }
            }
        }
    } else if (samples > 1) {
        int tileHeight = (height + TILE_SIZE - 1) / TILE_SIZE;
        int tileWidth = (width + TILE_SIZE - 1) / TILE_SIZE;
        #pragma omp parallel for schedule(dynamic, 1)
        for (int ty=0; ty<tileHeight; ty++) {
            for (int tx=0; tx<tileWidth; tx++) {
                if (shouldStopCheck(shouldStop)) continue;
                int yStart = ty * TILE_SIZE;
                int xStart = tx * TILE_SIZE;
                int yEnd = std::min(yStart + TILE_SIZE, height);
                int xEnd = std::min(xStart + TILE_SIZE, width);
                for (int y=yStart; y<yEnd; y++) {
                    for (int x=xStart; x<xEnd; x++) {
                        Color pixelColor{0, 0, 0};
                        for (int j=0; j<samples; j++) {
                            for (int i=0; i<samples; i++) {
                                double u = static_cast<double>(x) + (static_cast<double>(i) + 0.5) / samples;
                                double v = static_cast<double>(y) + (static_cast<double>(j) + 0.5) / samples;
                                Ray ray = cam.generateRay(u, v);
                                pixelColor = pixelColor + traceRay(ray, context.scene, MAX_DEPTH, nbAORays);
                            }
                        }
                        pixelBuffer[y * width + x] = pixelColor * (1.0 / (samples * samples));
                    }
                }
            }
        }
    }
}

void Renderer::writePPM(const std::string &outputPath, const std::vector<Color> &pixelBuffer, int width, int height) const {
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile)
        throw std::runtime_error("Error: cannot open output file '" + outputPath + "'");

    outFile << "P6\n" << width << ' ' << height << "\n255\n";

    std::vector<uint8_t> rgbData(width * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Color &pixelColor = pixelBuffer[y * width + x];
            int idx = (y * width + x) * 3;
            rgbData[idx] = static_cast<uint8_t>(toPPMByte(pixelColor.x));
            rgbData[idx + 1] = static_cast<uint8_t>(toPPMByte(pixelColor.y));
            rgbData[idx + 2] = static_cast<uint8_t>(toPPMByte(pixelColor.z));
        }
    }
    outFile.write(reinterpret_cast<const char*>(rgbData.data()), rgbData.size());

    outFile.close();
    if (outFile.fail())
        throw std::runtime_error("Write error on output file: " + outputPath);
}

void Renderer::displayLoop(std::vector<Color> &pixelBuffer,
    const RenderParams &params, const SceneContext &context, const std::atomic<bool> *externalStop) const {
    int width = params.width;
    int height = params.height;
    std::atomic<bool> localStop{false};
    std::atomic<bool> done{false};

    std::thread renderThread([&]() {
        renderTiles(context, pixelBuffer, params, &localStop);
        done.store(true, std::memory_order_release);
    });

    sf::Texture texture;
    texture.create(width, height);
    sf::Sprite sprite(texture);

    sf::RenderWindow window(sf::VideoMode(width, height), "Raytracer", sf::Style::Close);
    window.setFramerateLimit(30);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                localStop.store(true, std::memory_order_relaxed);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                localStop.store(true, std::memory_order_relaxed);
        }

        if (externalStop && externalStop->load(std::memory_order_relaxed))
            localStop.store(true, std::memory_order_relaxed);

        if (localStop.load(std::memory_order_relaxed))
            window.close();

        std::vector<uint8_t> rgbaData(width * height * 4);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Color &pixelColor = pixelBuffer[y * width + x];
                int idx = (y * width + x) * 4;
                rgbaData[idx] = static_cast<uint8_t>(toPPMByte(pixelColor.x));
                rgbaData[idx + 1] = static_cast<uint8_t>(toPPMByte(pixelColor.y));
                rgbaData[idx + 2] = static_cast<uint8_t>(toPPMByte(pixelColor.z));
                rgbaData[idx + 3] = 255;
            }
        }
        texture.update(rgbaData.data());

        window.clear();
        window.draw(sprite);
        window.display();
    }

    renderThread.join();
}

void Renderer::render(const SceneContext &context, const std::string &outputPath,
    bool display, const std::atomic<bool> *shouldStop) {
    if (!context.camera)
        throw std::runtime_error("No camera in scene context");
    const Camera &cam = *context.camera;

    RenderParams params;
    params.width = cam.getWidth();
    params.height = cam.getHeight();
    params.samples = (context.antialiasing) ? context.antialiasing->samples : 1;
    params.aaType = (context.antialiasing) ? context.antialiasing->type : "uniform";
    params.threshold = (context.antialiasing) ? context.antialiasing->threshold : 0.0;
    params.nbAORays = (context.nbAORays) ? *context.nbAORays : 16;

    std::vector<Color> pixelBuffer(params.width * params.height);

    context.scene.bvh();

    if (display)
        displayLoop(pixelBuffer, params, context, shouldStop);
    else
        renderTiles(context, pixelBuffer, params, shouldStop);

    writePPM(outputPath, pixelBuffer, params.width, params.height);
}

Color Renderer::traceRay(const Ray &ray, const Scene &scene, int depth, int nbAORays) const {
    auto hit = closestHit(ray, scene);
    if (!hit)
        return scene.backgroundColor();

    if (!hit->material)
        return scene.backgroundColor();
    ScatterResult scattered = hit->material->scatter(ray, *hit);

    Vec3 effectiveNormal = (scattered.modifiedNormal.has_value()) ? *scattered.modifiedNormal : hit->normal;

    Color lightDiffuse{0, 0, 0};
    Color lightSpecular{0, 0, 0};
    if (scattered.applyDirectLighting) {
        for (const auto &light : scene.lights()) {
            LightSample sample = light->getSample(hit->point, hit->normal);

            if (sample.isAmbient) {
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

            Color shadowFilter{1.0, 1.0, 1.0};
            {
                Ray sRay{hit->point + RayBias * hit->normal, sample.direction};
                double remaining = sample.distance;
                bool fullyBlocked = false;
                for (int si = 0; si < 8; si++) {
                    auto blocker = closestHit(sRay, scene);
                    if (!blocker || blocker->t >= remaining) break;
                    if (!blocker->material) { fullyBlocked = true; break; }
                    if (!blocker->material->isTransmissive()) { fullyBlocked = true; break; }
                    ScatterResult s = blocker->material->scatter(sRay, *blocker);
                    shadowFilter = shadowFilter * s.attenuation;
                    if (!s.scatteredRay) { fullyBlocked = true; break; }
                    remaining -= blocker->t;
                    sRay = *s.scatteredRay;
                }
                if (fullyBlocked) continue;
            }

            double diffuse = std::max(0.0, dot(effectiveNormal, sample.direction));
            lightDiffuse += sample.color * diffuse * shadowFilter;

            auto specularParams = hit->material->getSpecular();
            if (specularParams) {
                Vec3 refl = reflect(-sample.direction, effectiveNormal);
                Vec3 normalDir = normalize(-ray.direction);
                double specAngle = std::max(0.0, dot(refl, normalDir));
                double specular = std::pow(specAngle, specularParams->shininess);
                lightSpecular += specularParams->ks * sample.color * specular * shadowFilter;
            }
        }
    }

    Color result = scattered.attenuation * lightDiffuse + lightSpecular;
    if (depth > 0 && scattered.scatteredRay) {
        Color indirect = traceRay(*scattered.scatteredRay, scene, depth - 1, nbAORays);
        result += scattered.attenuation * indirect;
    }
    return result;
}
