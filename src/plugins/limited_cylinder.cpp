/*
 * Project: Raytracer
 * File name: limited_cylinder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Limited cylinder primitive plugin implementing ray-cylinder intersection via axis-projection.
 */

#include <cmath>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class LimitedCylinder : public IPrimitive {
public:
    LimitedCylinder() : _center(0, 0, 0), _axis(0, 1, 0), _height(0), _radius(0), _material(nullptr) {}
    ~LimitedCylinder() override = default;

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
        _height = params.at("h");
        if (_height <= 0.0)
            throw std::invalid_argument("Limited cylinder must have a height");
        _material = std::move(mat);
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        std::optional<HitRecord> best = intersectBody(ray);
        auto capBot = intersectDisk(ray, _center, -_axis, _radius);
        auto capTop = intersectDisk(ray, _center + _height * _axis, _axis, _radius);
        
        if (capBot && (!best || capBot->t < best->t))
            best = capBot;
        if (capTop && (!best || capTop->t < best->t))
            best = capTop;
        return best;
    }

private:
    Vec3 _center;
    Vec3 _axis;
    double _height;
    double _radius;
    std::shared_ptr<IMaterial> _material;

    std::optional<HitRecord> intersectBody(const Ray &ray) const {
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
            t = (-b + sqrtDiscr) / (2 * a);
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 radial = point - _center;
        double h = dot(radial, _axis);
        if (h < 0.0 || h > _height) return std::nullopt;

        Vec3 normal = radial - dot(radial, _axis) * _axis;
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;
        normal = normalize(normal);

        double u = 0.5 + std::atan2(normal.z, normal.x) / (2.0 * M_PI);
        if (u < 0.0)
            u += 1.0;
        double v = h / _height;
        Vec3 outwardRadial = normalize(radial - h * _axis);
        Vec3 tangent = normalize(cross(outwardRadial, _axis));
        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, _axis};
    }

    std::optional<HitRecord> intersectDisk(const Ray &ray, Vec3 center, Vec3 outwardNormal, double radius) const {
        double denom = dot(ray.direction, outwardNormal);
        if (std::abs(denom) < epsilon) return std::nullopt;

        double t = dot(center - ray.origin, outwardNormal) / denom;
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 diff = point - center;
        if (dot(diff, diff) > radius * radius) return std::nullopt;

        bool frontFace = denom < 0.0;
        Vec3 normal = frontFace ? outwardNormal : -outwardNormal;

        Vec3 up(0, 1, 0);
        Vec3 tangent = (std::abs(dot(up, outwardNormal)) > 0.999) ? Vec3(1, 0, 0) : normalize(cross(up, outwardNormal));
        Vec3 biTangent = normalize(cross(outwardNormal, tangent));

        double u = 0.5 + std::atan2(dot(diff, biTangent), dot(diff, tangent)) / (2.0 * M_PI);
        if (u < 0.0)
            u += 1.0;
        double v = std::sqrt(dot(diff, diff)) / radius;
        v -= std::floor(v);

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, biTangent};
    }
};

extern "C" IPrimitive *create() { return new LimitedCylinder(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
