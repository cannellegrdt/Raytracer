/*
 * Project: Raytracer
 * File name: ILight.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface definition for light sources in the ray tracer,
 *                   providing light sampling capabilities for illumination calculations.
 */

#ifndef ILIGHT_HPP_
    #define ILIGHT_HPP_
    #include "Vec3.hpp"
    #include "LightSample.hpp"

/// @brief Interface for light sources.
class ILight {
public:
    /// @brief Samples the light at a given point.
    /// @param hitPoint Point in space to sample from.
    /// @param normal Surface normal at hit point.
    /// @return LightSample with direction, color, and distance.
    virtual LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const = 0;
    virtual ~ILight() = default;
};

#endif /* ILIGHT_HPP_ */
