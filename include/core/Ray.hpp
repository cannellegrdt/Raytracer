/*
 * Project: Raytracer
 * File name: Ray.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ray struct with origin, direction, and parametric point evaluation.
 */

#ifndef RAY_HPP_
    #define RAY_HPP_
    #include "Vec3.hpp"

/// @brief Ray represented by origin and direction.
/// @details Used in raytracing to represent light rays that can intersect primitives.
struct Ray {
    Vec3 origin;    ///< Origin point of the ray.
    Vec3 direction; ///< Direction vector of the ray (not necessarily normalized).

    /// @brief Evaluates the ray at parameter t.
    /// @param t Parameter along the ray.
    /// @return Point on the ray at distance t from origin.
    Vec3 at(double t) const {
        return origin + t * direction;
    }
};

#endif /* RAY_HPP_ */
