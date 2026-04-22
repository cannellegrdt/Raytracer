/*
 * Project: Raytracer
 * File name: Reflection.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Material that reflects rays with a constant color attenuation.
 */

#ifndef REFLECTION_HPP_
    #define REFLECTION_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"

/// @brief Material that reflects rays with a constant color attenuation.
/// @details Implements reflective behavior where incoming rays are scattered
/// with a uniform color applied as attenuation.
class Reflection : public IMaterial {
public:
    /// @brief Constructs a Reflection material.
    /// @param color Reflected color (attenuation applied to scattered rays).
    explicit Reflection(const Color &color) : _color(color) {}

    /// @brief Computes the scattering of a ray hitting a surface.
    /// @param ray Incoming ray.
    /// @param hit Hit record with intersection details.
    /// @return ScatterResult containing attenuation color and scattered ray.
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    Color _color;  ///< Color attenuation applied to reflected rays.
};

#endif /* REFLECTION_HPP_ */
