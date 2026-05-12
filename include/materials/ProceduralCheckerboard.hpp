/*
 * Project: Raytracer
 * File name: ProceduralCheckerboard.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Procedural checkerboard material implementation that displays
 *                   alternating colors based on position using a 3D checkerboard pattern.
 */

#ifndef PROCEDURALCHECKERBOARD_HPP_
    #define PROCEDURALCHECKERBOARD_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"

/// @brief Material that displays a 3D procedural checkerboard pattern.
/// @details Alternates between two colors based on world-space position.
/// The pattern is computed using XOR of the discretized x, y, z coordinates.
class ProceduralCheckerboard : public IMaterial {
public:
    /// @brief Constructs a ProceduralCheckerboard material.
    /// @param colorA First checkerboard color.
    /// @param colorB Second checkerboard color.
    /// @param scale Size of each checkerboard cell.
    explicit ProceduralCheckerboard(const Color &colorA, const Color &colorB, double scale)
        : _colors(colorA, colorB), _scale(scale) {}

    /// @brief Computes the color based on checkerboard pattern at the hit point.
    /// @param ray Incoming ray (unused).
    /// @param hit Hit record with intersection details.
    /// @return ScatterResult containing only attenuation color (no scattered ray).
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    std::pair<Color, Color> _colors;  ///< Pair of colors (colorA, colorB) for the checkerboard pattern.
    double _scale;                    ///< Scale factor determining the size of each checkerboard cell.
};

#endif /* PROCEDURALCHECKERBOARD_HPP_ */
