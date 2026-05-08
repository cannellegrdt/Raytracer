/*
 * Project: Raytracer
 * File name: ProceduralMarble.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#include "ProceduralMarble.hpp"

ScatterResult ProceduralMarble::scatter(const Ray &/*ray*/, const HitRecord &hit) const {
    double noise = std::sin(_scale * hit.point.z + _turbulence * _noise.fractal(hit.point, _octaves));
    noise = (noise + 1.0) / 2.0;
    
    Color color = _colors.first * (1.0 - noise) + _colors.second * noise;
    return ScatterResult{color, std::nullopt};
}
