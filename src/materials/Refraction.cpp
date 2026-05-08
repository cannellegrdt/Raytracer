/*
 * Project: Raytracer
 * File name: Refraction.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implements refractive scattering with Snell-Descartes law and Schlick approximation.
 */

#include "Refraction.hpp"
#include "Common.hpp"
#include <cmath>
#include <random>

static ScatterResult reflection(Vec3 &dir, const HitRecord &hit, Color _color) {
    Vec3 reflected = normalize(dir - 2 * dot(dir, hit.normal) * hit.normal);
    Vec3 origin = hit.point + RayBias * hit.normal;
    return ScatterResult{_color, Ray{origin, reflected}};
}

ScatterResult Refraction::scatter(const Ray &ray, const HitRecord &hit) const {
    double etaRatio = (hit.frontFace) ? 1.0 / _ior : _ior;

    Vec3 dir = normalize(ray.direction);
    double cosTheta = std::fmin(1.0, std::fmax(0.0, dot(-dir, hit.normal)));
    double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

    if (etaRatio * sinTheta > 1.0)
        return reflection(dir, hit, _color);

    Vec3 rPerp = etaRatio * (dir + cosTheta * hit.normal);
    double rPerpLenSq = dot(rPerp, rPerp);
    Vec3 rParallel = -std::sqrt(1.0 - rPerpLenSq) * hit.normal;
    Vec3 refractedDir = normalize(rPerp + rParallel);

    double r0 = ((1 - _ior) / (1 + _ior)) * ((1 - _ior) / (1 + _ior));
    double reflectance = r0 + (1 - r0) * pow(1 - cosTheta, 5);
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0, 1);
    double nbRandom = dist(gen);
    if (nbRandom < reflectance)
        return reflection(dir, hit, _color);

    Vec3 origin = hit.point - RayBias * hit.normal;
    return ScatterResult{_color, Ray{origin, refractedDir}};
}