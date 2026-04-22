/*
 * Project: Raytracer
 * File name: AmbientLight.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of AmbientLight providing uniform illumination independent of surface orientation.
 */

#include "AmbientLight.hpp"

LightSample AmbientLight::getSample(const Vec3 &/*hitPoint*/, const Vec3 &/*normal*/) const {
    return LightSample{
        Vec3{0, 0, 0},
        _color * _intensity,
        std::numeric_limits<double>::infinity()
    };
}
