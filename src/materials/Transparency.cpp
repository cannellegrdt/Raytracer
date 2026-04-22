/*
 * Project: Raytracer
 * File name: Transparency.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#include "Transparency.hpp"

ScatterResult Transparency::scatter(const Ray &ray, const HitRecord &hit) const {
    Vec3 origin = hit.point - 1e-4 * hit.normal;
    return ScatterResult{_color, Ray{origin, ray.direction}};
}
