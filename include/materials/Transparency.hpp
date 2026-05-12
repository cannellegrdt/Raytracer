/*
 * Project: Raytracer
 * File name: Transparency.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Material implementation that transmits rays with color attenuation
 *                   for simulating transparent materials like glass or water.
 */

#ifndef TRANSPARENCY_HPP
    #define TRANSPARENCY_HPP
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"

/// @brief Material that transmits rays with color attenuation.
/// @details Implements transparent behavior where incoming rays pass through
/// the surface with color applied as attenuation.
class Transparency : public IMaterial {
public:
    /// @brief Constructs a Transparency material.
    /// @param color Transmission color (attenuation applied to transmitted rays).
    explicit Transparency(const Color &color) : _color(color) {}

    /// @brief Computes the scattering of a ray hitting a surface.
    /// @param ray Incoming ray.
    /// @param hit Hit record with intersection details.
    /// @return ScatterResult containing attenuation color and transmitted ray.
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;
    bool isTransmissive() const override { return true; }

private:
    Color _color;  ///< Color attenuation applied to transmitted rays.
};

#endif /* TRANSPARENCY_HPP */
