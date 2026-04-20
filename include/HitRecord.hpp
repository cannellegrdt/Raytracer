/*
 * Project: Raytracer
 * File name: HitRecord.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Intersection result holding hit distance, point, normal, material pointer, and face orientation.
 */

#ifndef HITRECORD_HPP_
    #define HITRECORD_HPP_
    #include "Vec3.hpp"

class IMaterial;

struct HitRecord {
    double t;
    Vec3 point;
    Vec3 normal;
    const IMaterial *material;
    bool frontFace;
};

#endif /* HITRECORD_HPP_ */
