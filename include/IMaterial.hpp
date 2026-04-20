/*
 * Project: Raytracer
 * File name: IMaterial.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for materials; defines how a ray scatters and attenuates on a surface.
 */

#ifndef IMATERIAL_HPP_
    #define IMATERIAL_HPP_
    #include "ScatterResult.hpp"
    #include "Ray.hpp"
    #include "HitRecord.hpp"

class IMaterial {
public:
    virtual ScatterResult scatter(const Ray &ray, const HitRecord &hit) const = 0;
    virtual ~IMaterial() = default;
};

#endif /* IMATERIAL_HPP_ */
