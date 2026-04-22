/*
 * Project: Raytracer
 * File name: Transparency.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#include "Transparency.hpp"
#include "Common.hpp"

ScatterResult Transparency::scatter(const Ray &ray, const HitRecord &hit) const {
    Vec3 origin = hit.point - RayBias * hit.normal;
    return ScatterResult{_color, Ray{origin, ray.direction}};
}
