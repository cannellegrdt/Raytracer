/*
 * Project: Raytracer
 * File name: cone.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Infinite cone primitive plugin implementing ray-cone intersection via quadratic form.
 */

#include <cmath>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class Cone : public IPrimitive {
public:
    Cone() : _apex(0, 0, 0), _axis(0, 1, 0), _angle(0), _material(nullptr) {}
    ~Cone() override = default;

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
        _material = std::move(mat);
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
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
            t = (-b + sqrtDiscr) / (2*a);
        if (t < epsilon) return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 radial = point - _apex;
        Vec3 normal = normalize(dot(radial, _axis) * _axis - k * radial);
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;
        
        return HitRecord{t, point, normal, _material, frontFace};
    };

private:
    Vec3 _apex;
    Vec3 _axis;
    double _angle;
    std::shared_ptr<IMaterial> _material;
};

extern "C" IPrimitive *create() { return new Cone(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
