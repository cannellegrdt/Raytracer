/*
 * Project: Raytracer
 * File name: PointLight.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of PointLight providing illumination from a point with quadratic attenuation.
 */

#include "PointLight.hpp"

LightSample PointLight::getSample(const Vec3 &hitPoint, const Vec3 &/*normal*/) const {
    Vec3 dir = _position - hitPoint;
    double lenDir = length(dir);

    if (lenDir < 1e-8) {
        return LightSample{
            Vec3(0, 0, 0),
            Vec3(0, 0, 0),
            lenDir
        };
    }

    return LightSample{
        normalize(dir),
        _color * _intensity / (lenDir * lenDir),
        lenDir
    };
}
