/*
 * Project: Raytracer
 * File name: ProceduralCheckerboard.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Procedural checkerboard material implementation that alternates between two colors based on world-space position.
 */

#include "ProceduralCheckerboard.hpp"

ScatterResult ProceduralCheckerboard::scatter(const Ray &/*ray*/, const HitRecord &hit) const {
    int ix = static_cast<int>(std::floor(hit.point.x / _scale));
    int iy = static_cast<int>(std::floor(hit.point.y / _scale));
    int iz = static_cast<int>(std::floor(hit.point.z / _scale));

    int iXOR = (ix & 1) ^ (iy & 1) ^ (iz & 1);

    if (iXOR == 0)
        return ScatterResult{_colors.first, std::nullopt};
    return ScatterResult{_colors.second, std::nullopt};
}
