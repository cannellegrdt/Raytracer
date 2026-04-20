/*
 * Project: Raytracer
 * File name: LightSample.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Light evaluation result holding direction toward the light, its color, and distance from the hit point.
 */

#ifndef LIGHTSAMPLE_HPP_
    #define LIGHTSAMPLE_HPP_
    #include "Vec3.hpp"

struct LightSample {
    Vec3 direction;
    Vec3 color;
    double distance;
};

#endif /* LIGHTSAMPLE_HPP_ */
