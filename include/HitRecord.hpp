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

/// @brief Records details of a ray-primitive intersection.
struct HitRecord {
    double t;                    ///< Distance from ray origin to intersection point.
    Vec3 point;                  ///< 3D position of the intersection.
    Vec3 normal;                 ///< Surface normal at the intersection.
    const IMaterial *material;   ///< Pointer to the material at the intersection.
    bool frontFace;              ///< True if ray hit front face of surface.
};

#endif /* HITRECORD_HPP_ */
