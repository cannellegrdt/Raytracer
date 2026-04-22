/*
 * Project: Raytracer
 * File name: Transparency.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#include "Transparency.hpp"

ScatterResult Transparency::scatter(const Ray &ray, const HitRecord &hit) const {
    return ScatterResult{_color, Ray{hit.point, ray.direction}};
}
