/*
 * Project: Raytracer
 * File name: plane.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Plane primitive plugin implementing ray-plane intersection.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

class Plane : public IPrimitive {
public:
    Plane() : _center(0, 0, 0), _normal(0, 1, 0), _material(nullptr) {}
    ~Plane() override = default;

    void configure(const std::unordered_map<std::string, double> &params,
        const IMaterial *mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        Vec3 n = { params.at("nx"), params.at("ny"), params.at("nz") };
        if (length(n) < epsilon)
            throw std::invalid_argument("Plane normal cannot be zero");
        _normal = normalize(n);
        _material = mat;
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        double parallel = dot(_normal, ray.direction);
        if (std::abs(parallel) < epsilon) return std::nullopt;

        double t = dot(_normal, _center - ray.origin) / parallel;
        if (t < epsilon) return std::nullopt;

        bool frontFace = parallel < 0.0;
        Vec3 normal = _normal;
        if (!frontFace)
            normal = -normal;
        return HitRecord{t, ray.at(t), normal, _material, frontFace};
    };

private:
    Vec3 _center;
    Vec3 _normal;
    const IMaterial *_material;
};

extern "C" IPrimitive *create() { return new Plane(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
