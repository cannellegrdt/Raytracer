/*
 * Project: Raytracer
 * File name: DirectionalLight.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of DirectionalLight providing illumination from a fixed direction at infinite distance.
 */

#include "DirectionalLight.hpp"

LightSample DirectionalLight::getSample(const Vec3 &/*hitPoint*/, const Vec3 &/*normal*/) const {
    return LightSample{
        -_direction,
        _color * _intensity,
        std::numeric_limits<double>::infinity()
    };
}
