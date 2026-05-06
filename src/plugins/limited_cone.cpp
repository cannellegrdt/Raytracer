/*
 * Project: Raytracer
 * File name: limited_cone.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Limited cone primitive plugin implementing ray-cone intersection via quadratic form.
 */

#include <cmath>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class LimitedCone : public IPrimitive {
public:
    LimitedCone() : _apex(0, 0, 0), _axis(0, 1, 0), _angle(0), _height(0), _material(nullptr) {}
    ~LimitedCone() override = default;

    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _apex = { params.at("x"), params.at("y"), params.at("z") };
        Vec3 axis = { params.at("ax"), params.at("ay"), params.at("az") };
        if (length(axis) < epsilon)
            throw std::invalid_argument("Cone axis cannot be zero");
        _axis = normalize(axis);
        _angle = params.at("angle");
        if (_angle <= 0.0 || _angle >= M_PI / 2.0)
            throw std::invalid_argument("Cone half-angle must be in (0, pi/2)");
        _height = params.at("h");
        if (_height <= 0.0)
            throw std::invalid_argument("Limited cone must have a height");
        _material = std::move(mat);
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        std::optional<HitRecord> best = intersectBody(ray);
        double capRadius = _height * std::tan(_angle);
        Vec3 capCenter = _apex + _height * _axis;
        
        auto cap = intersectDisk(ray, capCenter, _axis, capRadius);
        if (cap && (!best || cap->t < best->t))
            best = cap;
        return best;
    }

private:
    Vec3 _apex;
    Vec3 _axis;
    double _angle;
    double _height;
    std::shared_ptr<IMaterial> _material;

    std::optional<HitRecord> intersectBody(const Ray &ray) const {
        double k = std::cos(_angle) * std::cos(_angle);
        Vec3 vecOA = ray.origin - _apex;

        double dotDA = dot(ray.direction, _axis);
        double dotOA = dot(vecOA, _axis);
        double a = dotDA * dotDA - k;
        double b = 2.0 * (dotDA * dotOA - k * dot(ray.direction, vecOA));
        double c = dotOA * dotOA - k * dot(vecOA, vecOA);

        double discr = b * b - 4 * a * c;
        if (discr < 0.0) return std::nullopt;

        double sqrtDiscr = std::sqrt(discr);
        double t = (-b - sqrtDiscr) / (2 * a);
        if (t < epsilon)
            t = (-b + sqrtDiscr) / (2 * a);
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 radial = point - _apex;
        double h = dot(radial, _axis);
        if (h < epsilon || h > _height) return std::nullopt;

        Vec3 normal = normalize(dot(radial, _axis) * _axis - k * radial);
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;

        double u = 0.5 + std::atan2(normal.z, normal.x) / (2.0 * M_PI);
        if (u < 0.0)
            u += 1.0;
        double v = h / _height;

        return HitRecord{t, point, normal, _material, frontFace, {u, v}};
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
        Vec3 tangent = (std::abs(dot(up, outwardNormal)) > 0.999) ?  Vec3(1, 0, 0) : normalize(cross(up, outwardNormal));
        Vec3 bitangent = normalize(cross(outwardNormal, tangent));
        
        double u = 0.5 + std::atan2(dot(diff, bitangent), dot(diff, tangent)) / (2.0 * M_PI);
        if (u < 0.0) u += 1.0;
        double v = std::sqrt(dot(diff, diff)) / radius;
        v -= std::floor(v);

        return HitRecord{t, point, normal, _material, frontFace, {u, v}};
    }
};

extern "C" IPrimitive *create() { return new LimitedCone(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
