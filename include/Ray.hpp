/*
 * Project: Raytracer
 * File name: Ray.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ray struct.
 */

#ifndef RAY_HPP_
    #define RAY_HPP_
    #include "Vec3.hpp"

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Vec3 at(double t) const {
        return origin + t * direction;
    }
};

#endif /* RAY_HPP_ */
