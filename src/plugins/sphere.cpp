/*
 * Project: Raytracer
 * File name: sphere.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Sphere primitive plugin implementing ray-sphere intersection via the half-discriminant method.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Sphere primitive implementing ray-sphere intersection via the half-discriminant method.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Sphere : public IPrimitive {
public:
    /// @brief Default constructor initializing sphere to unit sphere at origin.
    Sphere() : _center(0, 0, 0), _radius(1.0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Sphere() override = default;

    /// @brief Configures the sphere with position, radius, and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "r" (radius).
    /// @param mat Shared pointer to the sphere material.
    /// @throws std::invalid_argument If radius is non-positive.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _radius = params.at("r");
        if (_radius <= 0.0)
            throw std::invalid_argument("Sphere radius must be > 0");
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box enclosing the sphere.
    AABB boundingBox() const override {
        Vec3 r(_radius, _radius, _radius);
        return AABB(_center - r, _center + r);
    }

    /// @brief Computes the nearest ray-sphere intersection.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 vecOC = ray.origin - _center;
        double a = dot(ray.direction, ray.direction);
        double c = dot(vecOC, vecOC) - _radius * _radius;
        
        double h = dot(vecOC, ray.direction);
        double discr = h * h - a * c;
        if (discr < 0.0) return std::nullopt;

        double sqrtDiscr = std::sqrt(discr);
        double t = (-h - sqrtDiscr) / a;
        if (t < epsilon)
            t = (-h + sqrtDiscr) / a;
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 normal = (point - _center) / _radius;
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;

        Vec3 pHat = (point - _center) / _radius;
        double u = 0.5 + std::atan2(pHat.z, pHat.x) / (2 * M_PI);
        double v = 0.5 - std::asin(pHat.y) / M_PI;

        Vec3 up(0, 1, 0);
        Vec3 tangent = (std::abs(pHat.y) > 0.999) ? Vec3(1, 0, 0) : normalize(cross(pHat, up));
        Vec3 bitangent = normalize(cross(pHat, tangent));

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, bitangent};
    };

private:
    Vec3 _center;                          ///< Center position of the sphere
    double _radius;                        ///< Radius of the sphere
    std::shared_ptr<IMaterial> _material;  ///< Material of the sphere
};

extern "C" IPrimitive *create() { return new Sphere(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
