/*
 * Project: Raytracer
 * File name: IMaterial.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for materials; defines how a ray scatters and attenuates on a surface.
 */

#ifndef IMATERIAL_HPP_
    #define IMATERIAL_HPP_
    #include <optional>
    #include "ScatterResult.hpp"
    #include "Ray.hpp"
    #include "HitRecord.hpp"
    #include "SpecularParams.hpp"

    /// @brief Interface for material types.
    /// @details Defines how rays interact with surfaces (scatter, reflect, refract, absorb).
    class IMaterial {
    public:
        /// @brief Computes the scattering of a ray hitting a surface.
        /// @param ray Incoming ray.
        /// @param hit Hit record with intersection details.
        /// @return ScatterResult containing attenuation color and scattered ray (if any).
        virtual ScatterResult scatter(const Ray &ray, const HitRecord &hit) const = 0;

        /// @brief Returns specular parameters for Phong shading, if supported by the material.
        /// @return Optional SpecularParams (nullopt if material doesn't support specular highlights).
        virtual std::optional<SpecularParams> getSpecular() const { return std::nullopt; }

        /// @brief Returns true if the material transmits light (e.g. transparency), allowing shadow rays to pass through.
        virtual bool isTransmissive() const { return false; }

        virtual ~IMaterial() = default;
    };

#endif /* IMATERIAL_HPP_ */
