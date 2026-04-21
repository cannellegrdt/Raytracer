/*
 * Project: Raytracer
 * File name: sphere.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Sphere primitive plugin implementing ray-sphere intersection via the half-discriminant method.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class Sphere : public IPrimitive {
public:
    Sphere() : _center(0, 0, 0), _radius(1.0), _material(nullptr) {}
    ~Sphere() override = default;

    void configure(const std::unordered_map<std::string, double> &params,
        const IMaterial *mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _radius = params.at("r");
        if (_radius <= 0.0)
            throw std::invalid_argument("Sphere radius must be > 0");
        _material = mat;
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 vecOC = ray.origin - _center;
        double a = dot(ray.direction, ray.direction);
        double c = dot(vecOC, vecOC) - _radius * _radius;
        
        double h = dot(vecOC, ray.direction);
        double discr = h * h - a * c;
        if (discr < 0.0) return std::nullopt;

        double sqrt_discr = std::sqrt(discr);
        double t = (-h - sqrt_discr) / a;
        if (t < epsilon)
            t = (-h + sqrt_discr) / a;
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 normal = (point - _center) / _radius;
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;
        
        return HitRecord(t, point, normal, _material, frontFace);
    };

private:
    Vec3 _center;
    double _radius;
    const IMaterial *_material;
};

extern "C" IPrimitive *create() { return new Sphere(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
