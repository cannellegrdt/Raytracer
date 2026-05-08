/*
 * Project: Raytracer
 * File name: Transparency.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Transparent material allowing rays to pass through with optional color filtering.
 */

#include "Transparency.hpp"
#include "Common.hpp"

ScatterResult Transparency::scatter(const Ray &ray, const HitRecord &hit) const {
    Vec3 origin = hit.point - RayBias * hit.normal;
    Color attenuation = hit.frontFace ? _color : Color{1.0, 1.0, 1.0};
    
    return ScatterResult{attenuation, Ray{origin, ray.direction}, std::nullopt};
}
