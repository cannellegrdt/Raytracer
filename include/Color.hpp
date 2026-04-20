/*
 * Project: Raytracer
 * File name: Color.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Color alias over Vec3, with component clamping and PPM byte conversion.
 */

#ifndef COLOR_HPP_
    #define COLOR_HPP_
    #include <algorithm>
    #include "Vec3.hpp"

    constexpr double ColorScale = 255.999;

using Color = Vec3;

inline Color clamp(Color c, double min = 0.0, double max = 1.0) {
    return Color(
        std::clamp(c.x, min, max),
        std::clamp(c.y, min, max),
        std::clamp(c.z, min, max)
    );
}

inline int toPPMByte(double component) {
    double clamped = std::clamp(component, 0.0, 1.0);
    return static_cast<int>(ColorScale * clamped);
}

#endif /* COLOR_HPP_ */
