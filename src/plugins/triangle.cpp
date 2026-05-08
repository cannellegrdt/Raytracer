/*
 * Project: Raytracer
 * File name: triangle.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Triangle primitive plugin implementing ray-triangle intersection via Möller-Trumbore algorithm.
 */

#include <algorithm>
#include <cmath>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Triangle primitive implementing ray-triangle intersection via Möller-Trumbore algorithm.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Triangle : public IPrimitive {
public:
    /// @brief Default constructor initializing triangle vertices and edges to default values.
    Triangle() : _v0(0, 0, 0), _v1(0, 0, 0), _v2(0, 0, 0),
        _edge1(0, 0, 0), _edge2(0, 0, 0), _normal(0, 0, 0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Triangle() override = default;

    /// @brief Configures the triangle with three vertices and material.
    /// @param params Unordered map of parameters: "v0x", "v0y", "v0z", "v1x", "v1y", "v1z", "v2x", "v2y", "v2z" (vertex coordinates).
    /// @param mat Shared pointer to the triangle material.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _v0 = { params.at("v0x"), params.at("v0y"), params.at("v0z") };
        _v1 = { params.at("v1x"), params.at("v1y"), params.at("v1z") };
        _v2 = { params.at("v2x"), params.at("v2y"), params.at("v2z") };
        _edge1 = _v1 - _v0;
        _edge2 = _v2 - _v0;
        _normal = normalize(cross(_edge1, _edge2));
        _material = std::move(mat);
    }

    /// @brief Computes the nearest ray-triangle intersection using Möller-Trumbore algorithm.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 p = cross(ray.direction, _edge2);
        double det = dot(_edge1, p);
        if (fabs(det) < epsilon) return std::nullopt;

        Vec3 q = ray.origin - _v0;
        double u = dot(q, p) / det;
        if (u < 0.0 || u > 1.0) return std::nullopt;

        Vec3 r = cross(q, _edge1);
        double v = dot(ray.direction, r) / det;
        if (v < 0.0 || u + v > 1.0) return std::nullopt;

        double t = dot(_edge2, r) / det;
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        bool frontFace = dot(ray.direction, _normal) < 0;
        Vec3 normal = _normal;
        if (!frontFace)
            normal = -normal;

        Vec3 tangent = normalize(_edge1);
        Vec3 bitangent = normalize(cross(normal, tangent));

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, bitangent};
    }

private:
    Vec3 _v0;                               ///< First vertex of the triangle
    Vec3 _v1;                               ///< Second vertex of the triangle
    Vec3 _v2;                               ///< Third vertex of the triangle
    Vec3 _edge1;                            ///< First edge vector (v1 - v0)
    Vec3 _edge2;                            ///< Second edge vector (v2 - v0)
    Vec3 _normal;                           ///< Precomputed triangle normal
    std::shared_ptr<IMaterial> _material;   ///< Material of the triangle
};

extern "C" IPrimitive *create() { return new Triangle(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
