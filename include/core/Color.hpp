/*
 * Project: Raytracer
 * File name: Color.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Color type alias over Vec3 with component clamping utilities
 *                   and PPM byte conversion functions for image output.
 */

#ifndef COLOR_HPP_
    #define COLOR_HPP_
    #include <algorithm>
    #include "Vec3.hpp"

    constexpr double ColorScale = 255.999; ///< Scale factor for converting float color [0,1] to byte [0,255].

using Color = Vec3; ///< Color is a 3D vector with r, g, b components.

/// @brief Clamps color components to a range.
/// @param c Color to clamp.
/// @param min Minimum value (default 0.0).
/// @param max Maximum value (default 1.0).
/// @return New color with clamped components.
inline Color clamp(Color c, double min = 0.0, double max = 1.0) {
    return Color(
        std::clamp(c.x, min, max),
        std::clamp(c.y, min, max),
        std::clamp(c.z, min, max)
    );
}

/// @brief Converts a color component to a PPM byte value.
/// @param component Color component in range [0, 1].
/// @return Integer in range [0, 255].
inline int toPPMByte(double component) {
    double clamped = std::clamp(component, 0.0, 1.0);
    return static_cast<int>(ColorScale * clamped);
}

#endif /* COLOR_HPP_ */
