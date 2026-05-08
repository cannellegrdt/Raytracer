/*
 * Project: Raytracer
 * File name: cylinder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Infinite cylinder primitive plugin implementing ray-cylinder intersection via axis-projection.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Infinite cylinder primitive implementing ray-cylinder intersection via axis-projection.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Cylinder : public IPrimitive {
public:
    /// @brief Default constructor initializing cylinder parameters to default values.
    Cylinder() : _center(0, 0, 0), _axis(0, 1, 0), _radius(0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Cylinder() override = default;

    /// @brief Configures the infinite cylinder with position, orientation, radius, and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "ax", "ay", "az" (axis), "r" (radius).
    /// @param mat Shared pointer to the cylinder material.
    /// @throws std::invalid_argument If axis is zero or radius is non-positive.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        Vec3 axis = { params.at("ax"), params.at("ay"), params.at("az") };
        if (length(axis) < epsilon)
            throw std::invalid_argument("Cylinder axis cannot be zero");
        _axis = normalize(axis);
        _radius = params.at("r");
        if (_radius <= 0.0)
            throw std::invalid_argument("Cylinder radius must be > 0");
        _material = std::move(mat);
    }

    /// @brief Computes the nearest ray-cylinder intersection (infinite cylinder).
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 dPrime = ray.direction - dot(ray.direction, _axis) * _axis;
        Vec3 vecOCPrime = (ray.origin - _center) - dot(ray.origin - _center, _axis) * _axis;

        double a = dot(dPrime, dPrime);
        if (std::abs(a) < epsilon) return std::nullopt;
        double b = 2.0 * dot(dPrime, vecOCPrime);
        double c = dot(vecOCPrime, vecOCPrime) - _radius * _radius;

        double discr = b * b - 4 * a * c;
        if (discr < 0.0) return std::nullopt;

        double sqrtDiscr = std::sqrt(discr);
        double t = (-b - sqrtDiscr) / (2 * a);
        if (t < epsilon)
            t = (-b + sqrtDiscr) / (2*a);
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 radial = point - _center;
        double h = dot(radial, _axis);
        Vec3 normal = radial - h * _axis;
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;
        normal = normalize(normal);

        double u = 0.5 + std::atan2(normal.z, normal.x) / (2.0 * M_PI);
        if (u < 0.0)
            u += 1.0;
        
        double v = h * 0.1;
        v -= std::floor(v);
        if (v < 0.0)
            v += 1.0;

        Vec3 outwardRadial = normalize(radial - h * _axis);
        Vec3 tangent = normalize(cross(outwardRadial, _axis));
        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, _axis};
    };

private:
    Vec3 _center;                          ///< Reference center point on the cylinder axis
    Vec3 _axis;                            ///< Axis direction of the cylinder
    double _radius;                        ///< Radius of the cylinder
    std::shared_ptr<IMaterial> _material;  ///< Material of the cylinder
};

extern "C" IPrimitive *create() { return new Cylinder(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
