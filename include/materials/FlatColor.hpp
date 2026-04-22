/*
 * Project: Raytracer
 * File name: FlatColor.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Flat-color material returning a constant color attenuation with no scattered ray.
 */

#ifndef FLATCOLOR_HPP_
    #define FLATCOLOR_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"

/// @brief Flat-color material returning a constant color attenuation.
/// @details Returns a constant color with no scattered ray, useful for ambient
/// or emissive-like materials that don't reflect light in a traditional sense.
class FlatColor : public IMaterial {
public:
    /// @brief Constructs a FlatColor material.
    /// @param color Constant color to return.
    explicit FlatColor(const Color &color) : _color(color) {}

    /// @brief Computes the scattering of a ray hitting a surface.
    /// @param ray Incoming ray.
    /// @param hit Hit record with intersection details.
    /// @return ScatterResult containing constant color attenuation with no scattered ray.
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    Color _color;  ///< Constant color to return.
};

#endif /* FLATCOLOR_HPP_ */
