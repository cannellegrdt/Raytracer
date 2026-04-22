/*
 * Project: Raytracer
 * File name: LightSample.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Light evaluation result holding direction toward the light, its color, and distance from the hit point.
 */

#ifndef LIGHTSAMPLE_HPP_
    #define LIGHTSAMPLE_HPP_
    #include "Vec3.hpp"

/// @brief Result of sampling a light at a point.
struct LightSample {
    Vec3 direction;       ///< Direction from hit point toward the light.
    Vec3 color;           ///< Color/intensity of the light.
    double distance;      ///< Distance from hit point to light source.
    bool isAmbient = false; ///< True for ambient lights (no direction, no shadow test).
};

#endif /* LIGHTSAMPLE_HPP_ */
