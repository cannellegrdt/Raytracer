/*
 * Project: Raytracer
 * File name: cube.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Axis-aligned cube primitive plugin implementing ray-cube intersection via slab method.
 */

#include <limits>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Axis-aligned cube primitive implementing ray-cube intersection via slab method.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Cube : public IPrimitive {
public:
    /// @brief Default constructor initializing cube bounds to unit cube at origin.
    Cube() : _min(0, 0, 0), _max(1, 1, 1), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Cube() override = default;

    /// @brief Configures the cube with center position, size, and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "s" (size of cube side).
    /// @param mat Shared pointer to the cube material.
    /// @throws std::invalid_argument If size is non-positive.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        Vec3 center = { params.at("x"), params.at("y"), params.at("z") };
        double size = params.at("s");
        if (size <= 0.0)
            throw std::invalid_argument("Cube size must be > 0");
        double halfSize = size / 2.0;
        _min = center - Vec3(halfSize, halfSize, halfSize);
        _max = center + Vec3(halfSize, halfSize, halfSize);
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box defined by the cube's min and max points.
    AABB boundingBox() const override { return AABB(_min, _max); }

    /// @brief Computes the nearest ray-cube intersection using slab method.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        double tMin = 0.0;
        double tMax = std::numeric_limits<double>::infinity();
        if (!AABB(_min, _max).intersect(ray, tMin, tMax))
            return std::nullopt;

        double t = (tMin > epsilon) ? tMin : tMax;
        if (t < epsilon)
            return std::nullopt;

        Vec3 point = ray.at(t);
        Vec3 normal = computeNormal(point);
        bool frontFace = dot(ray.direction, normal) < 0.0;
        if (!frontFace)
            normal = -normal;

        double u = (point.x - _min.x) / (_max.x - _min.x);
        double v = (point.y - _min.y) / (_max.y - _min.y);

        Vec3 tangent(1, 0, 0);
        Vec3 bitangent(0, 1, 0);

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, tangent, bitangent};
    };

private:
    Vec3 _min;                              ///< Minimum corner of the axis-aligned cube
    Vec3 _max;                              ///< Maximum corner of the axis-aligned cube
    std::shared_ptr<IMaterial> _material;   ///< Material of the cube

    /// @brief Computes the surface normal at a given point on the cube.
    /// @param point The intersection point on the cube surface.
    /// @return Surface normal vector pointing outward.
    Vec3 computeNormal(const Vec3 &point) const {
        double eps = 1e-6;
        if (std::abs(point.x - _min.x) < eps) return Vec3(-1, 0, 0);
        if (std::abs(point.x - _max.x) < eps) return Vec3(1, 0, 0);
        if (std::abs(point.y - _min.y) < eps) return Vec3(0, -1, 0);
        if (std::abs(point.y - _max.y) < eps) return Vec3(0, 1, 0);
        if (std::abs(point.z - _min.z) < eps) return Vec3(0, 0, -1);
        if (std::abs(point.z - _max.z) < eps) return Vec3(0, 0, 1);
        return Vec3(0, 1, 0);
    }
};

extern "C" IPrimitive *create() { return new Cube(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
