/*
 * Project: Raytracer
 * File name: Reflection.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Reflection material that scatters rays with specular reflection.
 */

#include "Reflection.hpp"
#include "Common.hpp"

ScatterResult Reflection::scatter(const Ray &ray, const HitRecord &hit) const {
    Vec3 dir = normalize(ray.direction);
    Vec3 reflected = normalize(dir - 2 * dot(dir, hit.normal) * hit.normal);
    Vec3 origin = hit.point + RayBias * hit.normal;
    return ScatterResult{_color, Ray{origin, reflected}, std::nullopt, false};
}
