/*
 * Project: Raytracer
 * File name: Refraction.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Material that refracts rays according to Snell-Descartes law.
 */

#ifndef REFRACTION_HPP_
    #define REFRACTION_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"

/// @brief Material that refracts rays according to Snell-Descartes law.
/// @details Implements refractive behavior using Snell's law (n₁·sin(θ₁) = n₂·sin(θ₂)).
///          Handles total internal reflection and uses Schlick's approximation for Fresnel effects.
///          Common IOR values: air ≈ 1.0, water ≈ 1.33, glass ≈ 1.5.
class Refraction : public IMaterial {
public:
    /// @brief Constructs a Refraction material.
    /// @param color Color attenuation applied to refracted rays.
    /// @param ior Index of refraction (must be > 0, e.g., 1.5 for glass).
    explicit Refraction(const Color &color, double ior) : _color(color), _ior(ior) {}

    /// @brief Computes the scattering of a ray hitting a surface.
    /// @param ray Incoming ray.
    /// @param hit Hit record with intersection details (including frontFace for inside/outside test).
    /// @return ScatterResult containing attenuation color and refracted ray (or reflected ray on total internal reflection).
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    Color _color;  ///< Color attenuation applied to refracted rays.
    double _ior;   ///< Index of refraction (IOR) of the material.
};

#endif /* REFRACTION_HPP_ */
