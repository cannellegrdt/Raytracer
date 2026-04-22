/*
 * Project: Raytracer
 * File name: cylinder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Infinite cylinder primitive plugin implementing ray-cylinder intersection via axis-projection.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class Cylinder : public IPrimitive {
public:
    Cylinder() : _center(0, 0, 0), _axis(0, 1, 0), _radius(0), _material(nullptr) {}
    ~Cylinder() override = default;

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

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 dPrime = ray.direction - dot(ray.direction, _axis) * _axis;
        Vec3 vecOCPrime = (ray.origin - _center) - dot(ray.origin - _center, _axis) * _axis;

        double a = dot(dPrime, dPrime);
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
        Vec3 normal = radial - dot(radial, _axis) * _axis;
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;

        return HitRecord{t, point, normal, _material, frontFace};
    };

private:
    Vec3 _center;
    Vec3 _axis;
    double _radius;
    std::shared_ptr<IMaterial> _material;
};

extern "C" IPrimitive *create() { return new Cylinder(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
