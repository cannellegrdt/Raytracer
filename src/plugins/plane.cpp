/*
 * Project: Raytracer
 * File name: plane.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Plane primitive plugin implementing ray-plane intersection.
 */

#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Plane primitive implementing ray-plane intersection.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Plane : public IPrimitive {
public:
    /// @brief Default constructor initializing plane at origin with upward normal.
    Plane() : _center(0, 0, 0), _normal(0, 1, 0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Plane() override = default;

    /// @brief Configures the plane with position, normal, and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (point on plane), "nx", "ny", "nz" (normal direction).
    /// @param mat Shared pointer to the plane material.
    /// @throws std::invalid_argument If normal is zero.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        Vec3 n = { params.at("nx"), params.at("ny"), params.at("nz") };
        if (length(n) < epsilon)
            throw std::invalid_argument("Plane normal cannot be zero");
        _normal = normalize(n);
        _material = std::move(mat);
    }

    /// @brief Returns an infinite bounding box for the infinite plane.
    AABB boundingBox() const override { return AABB::infinite(); }

    /// @brief Computes the ray-plane intersection.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        double parallel = dot(_normal, ray.direction);
        if (std::abs(parallel) < epsilon) return std::nullopt;

        double t = dot(_normal, _center - ray.origin) / parallel;
        if (t < epsilon) return std::nullopt;

        bool frontFace = parallel < 0.0;
        Vec3 normal = _normal;
        if (!frontFace)
            normal = -normal;

        Vec3 point = ray.at(t);
        Vec3 radial = point - _center;

        Vec3 up(0, 1, 0);
        Vec3 tan1 = (std::abs(dot(up, _normal)) > 0.999) ? Vec3(1, 0, 0) : normalize(cross(up, _normal));
        Vec3 tan2 = normalize(cross(_normal, tan1));
        double u = dot(radial, tan1);
        u -= std::floor(u);
        double v = dot(radial, tan2);
        v -= std::floor(v);

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tan1, tan2};
    };

private:
    Vec3 _center;                          ///< A point on the plane
    Vec3 _normal;                          ///< Normal vector of the plane (unit length)
    std::shared_ptr<IMaterial> _material;  ///< Material of the plane
};

extern "C" IPrimitive *create() { return new Plane(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
