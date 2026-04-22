/*
 * Project: Raytracer
 * File name: AmbientLight.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ambient light source that illuminates evenly from all directions.
 */

#ifndef AMBIENTLIGHT_HPP_
    #define AMBIENTLIGHT_HPP_
    #include "ILight.hpp"
    #include "Color.hpp"

/// @brief Ambient light source that provides uniform illumination from all directions.
class AmbientLight : public ILight {
public:
    /// @brief Constructs an AmbientLight with the given color and intensity.
    /// @param color Color of the ambient light.
    /// @param intensity Intensity multiplier for the light.
    AmbientLight(const Color &color, double intensity) : _color(color), _intensity(intensity) {};

    /// @brief Samples the ambient light at a given point.
    /// @param hitPoint Point in space to sample from (unused for ambient light).
    /// @param normal Surface normal at hit point (unused for ambient light).
    /// @return LightSample with direction, color, and distance.
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Color _color;      ///< Color of the ambient light.
    double _intensity; ///< Intensity multiplier for the light.
};

#endif /* AMBIENTLIGHT_HPP_ */
