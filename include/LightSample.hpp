/*
 * Project: Raytracer
 * File name: LightSample.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: LightSample struct.
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
