/*
 * Project: Raytracer
 * File name: ScatterResult.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: ScatterResult struct.
 */

#ifndef SCATTERRESULT_HPP_
    #define SCATTERRESULT_HPP_
    #include <optional>
    #include "Vec3.hpp"
    #include "Ray.hpp"

struct ScatterResult {
    Vec3 attenuation;
    std::optional<Ray> scatteredRay;
};

#endif /* SCATTERRESULT_HPP_ */
