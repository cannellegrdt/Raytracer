/*
 * Project: Raytracer
 * File name: ScatterResult.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Material scatter output holding color attenuation and an optional scattered ray.
 */

#ifndef SCATTERRESULT_HPP_
    #define SCATTERRESULT_HPP_
    #include <optional>
    #include "Vec3.hpp"
    #include "Ray.hpp"

/// @brief Result of a material's scatter operation.
struct ScatterResult {
    Vec3 attenuation;                   ///< Color attenuation (how much light is reflected/transmitted).
    std::optional<Ray> scatteredRay;    ///< New ray after scattering (empty if absorbed).
    std::optional<Vec3> modifiedNormal; ///< Optional modified normal (e.g., from normal mapping) for lighting calculations.
};

#endif /* SCATTERRESULT_HPP_ */
