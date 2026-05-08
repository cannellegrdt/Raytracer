/*
 * Project: Raytracer
 * File name: FlatColor.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Flat-color material implementation returning constant color attenuation.
 */

#include "FlatColor.hpp"

ScatterResult FlatColor::scatter(const Ray &/*ray*/, const HitRecord &/*hit*/) const {
    return ScatterResult{_color, std::nullopt, std::nullopt};
}
