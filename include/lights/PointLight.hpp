/*
 * Project: Raytracer
 * File name: PointLight.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Point light source that illuminates from a specific position with quadratic attenuation.
 */

#ifndef POINTLIGHT_HPP_
    #define POINTLIGHT_HPP_
    #include "ILight.hpp"
    #include "Color.hpp"

/// @brief Point light source that provides illumination from a specific position in space.
class PointLight : public ILight {
public:
    /// @brief Constructs a PointLight with the given position, color, and intensity.
    /// @param position Position of the point light in world space.
    /// @param color Color of the light.
    /// @param intensity Intensity multiplier for the light.
    PointLight(const Vec3 &position, const Color &color, double intensity) : _position(position), _color(color), _intensity(intensity) {};

    /// @brief Samples the point light at a given point.
    /// @param hitPoint Point in space to sample from.
    /// @param normal Surface normal at hit point (unused for point light).
    /// @return LightSample with direction, color (with attenuation), and distance.
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Vec3 _position;    ///< Position of the point light in world space.
    Color _color;      ///< Color of the point light.
    double _intensity; ///< Intensity multiplier for the light.
};

#endif /* POINTLIGHT_HPP_ */
