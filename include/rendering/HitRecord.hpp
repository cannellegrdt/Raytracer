/*
 * Project: Raytracer
 * File name: HitRecord.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Intersection result holding hit distance, point, normal, material pointer, and face orientation.
 */

#ifndef HITRECORD_HPP_
    #define HITRECORD_HPP_
    #include <memory>
    #include "Vec3.hpp"

class IMaterial;

/// @brief Records details of a ray-primitive intersection.
struct HitRecord {
    double t;                                  ///< Distance from ray origin to intersection point.
    Vec3 point;                                ///< 3D position of the intersection.
    Vec3 normal;                               ///< Surface normal at the intersection.
    std::shared_ptr<IMaterial> material;       ///< Shared pointer to the material at the intersection.
    bool frontFace;                            ///< True if ray hit front face of surface.
    std::pair<double, double> UV = {0.0, 0.0}; ///< UV texture coordinates [0, 1].
    Vec3 tangent{};                             ///< Tangent vector for normal mapping.
    Vec3 bitangent{};                           ///< Bitangent vector for normal mapping.
};

#endif /* HITRECORD_HPP_ */
