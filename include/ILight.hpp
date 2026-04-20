/*
 * Project: Raytracer
 * File name: ILight.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for light sources; provides a sample (direction, color, distance) at a hit point.
 */

#ifndef ILIGHT_HPP_
    #define ILIGHT_HPP_
    #include "Vec3.hpp"
    #include "LightSample.hpp"

class ILight {
public:
    virtual LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const = 0;
    virtual ~ILight() = default;
};

#endif /* ILIGHT_HPP_ */
