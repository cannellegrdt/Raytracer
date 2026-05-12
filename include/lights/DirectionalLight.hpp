/*
 * Project: Raytracer
 * File name: DirectionalLight.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Directional light source implementation that illuminates
 *                   from a constant direction, simulating distant light sources
 *                   like the sun with parallel light rays.
 */

#ifndef DIRECTIONALLIGHT_HPP_
    #define DIRECTIONALLIGHT_HPP_
    #include "ILight.hpp"
    #include "Color.hpp"

/// @brief Directional light source that provides illumination from a constant direction.
class DirectionalLight : public ILight {
public:
    /// @brief Constructs a DirectionalLight with the given direction, color, and intensity.
    /// @param direction Direction of the light (normalized).
    /// @param color Color of the light.
    /// @param intensity Intensity multiplier for the light.
    DirectionalLight(const Vec3 &direction, const Color &color, double intensity)
        : _direction(normalize(direction)), _color(color), _intensity(intensity) {};

    /// @brief Samples the directional light at a given point.
    /// @param hitPoint Point in space to sample from.
    /// @param normal Surface normal at hit point.
    /// @return LightSample with direction, color, and distance.
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Vec3 _direction;    ///< Direction of the light.
    Color _color;       ///< Color of the light.
    double _intensity;  ///< Intensity multiplier for the light.
};

#endif /* DIRECTIONALLIGHT_HPP_ */
