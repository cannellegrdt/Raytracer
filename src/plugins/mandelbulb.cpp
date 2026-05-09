/*
 * Project: Raytracer
 * File name: mandelbulb.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Mandelbulb fractal primitive plugin implementing ray-fractal intersection via distance estimation and ray marching.
 */

#include <interfaces/IPrimitive.hpp>
#include <rendering/HitRecord.hpp>
#include <core/Vec3.hpp>
#include <core/AABB.hpp>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <utility>

/// @brief Mandelbulb fractal primitive implementing ray-fractal intersection via distance estimation and ray marching.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Mandelbulb : public IPrimitive {
public:
    /// @brief Default constructor initializing Mandelbulb parameters to default values.
    Mandelbulb() : _center(0, 0, 0), _scale(1.0), _power(8.0), _nbIters(20), _bailout(2.0),
        _epsilon(1e-5), _maxSteps(1000) {}

    /// @brief Configures the Mandelbulb with parameters and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "s" (scale), "power" (fractal power), "iters" (iterations), "bailout" (bailout radius).
    /// @param mat Shared pointer to the Mandelbulb material.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _scale = params.at("s");
        _power = params.at("power");
        _nbIters = static_cast<int>(params.at("iters"));
        _bailout = params.at("bailout");
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box enclosing the Mandelbulb.
    AABB boundingBox() const override {
        double radius = _scale * _bailout;
        return AABB(_center - Vec3(radius, radius, radius), _center + Vec3(radius, radius, radius));
    }

    /// @brief Computes the nearest ray-Mandelbulb intersection using distance estimation and ray marching.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        AABB bbox = boundingBox();
        auto bboxIntersect = rayAABBIntersect(ray, bbox);
        if (!bboxIntersect) return std::nullopt;

        double t_start_world = bboxIntersect->first;
        double t_end_world = bboxIntersect->second;

        Vec3 ro = (ray.origin - _center) / _scale;
        Vec3 rd = normalize(ray.direction / _scale);

        double t = t_start_world / _scale;
        for (int i=0; i<_maxSteps; i++) {
            Vec3 p = ro + rd * t;
            double d = estimateDist(p);
            if (d < _epsilon * t) {
                Vec3 point = _center + p * _scale;
                Vec3 normal = normalize(gradient(p));
                double u = std::atan2(p.y, p.x) / (2.0 * M_PI) + 0.5;
                double v = std::acos(p.z / length(p)) / M_PI;
                Vec3 tangent = normalize(Vec3(-normal.y, normal.x, 0));
                Vec3 bitangent = cross(normal, tangent);

                return HitRecord{t * _scale, point, normal, _material, true, {u, v}, tangent, bitangent};
            }
            t += d;
            if (t > t_end_world / _scale)
                break;
        }
        return std::nullopt;
    }

private:
    /// @brief Computes ray-AABB intersection in world space.
    /// @param ray The ray.
    /// @param bbox The AABB.
    /// @return Optional pair of (t_near, t_far) if intersects, else nullopt.
    std::optional<std::pair<double, double>> rayAABBIntersect(const Ray& ray, const AABB& bbox) const {
        double tmin = -std::numeric_limits<double>::infinity();
        double tmax = std::numeric_limits<double>::infinity();
        if (std::abs(ray.direction.x) < 1e-8) {
            if (ray.origin.x < bbox.min.x || ray.origin.x > bbox.max.x) return std::nullopt;
        } else {
            double t1 = (bbox.min.x - ray.origin.x) / ray.direction.x;
            double t2 = (bbox.max.x - ray.origin.x) / ray.direction.x;
            tmin = std::max(tmin, std::min(t1, t2));
            tmax = std::min(tmax, std::max(t1, t2));
        }

        if (std::abs(ray.direction.y) < 1e-8) {
            if (ray.origin.y < bbox.min.y || ray.origin.y > bbox.max.y) return std::nullopt;
        } else {
            double t1 = (bbox.min.y - ray.origin.y) / ray.direction.y;
            double t2 = (bbox.max.y - ray.origin.y) / ray.direction.y;
            tmin = std::max(tmin, std::min(t1, t2));
            tmax = std::min(tmax, std::max(t1, t2));
        }

        if (std::abs(ray.direction.z) < 1e-8) {
            if (ray.origin.z < bbox.min.z || ray.origin.z > bbox.max.z) return std::nullopt;
        } else {
            double t1 = (bbox.min.z - ray.origin.z) / ray.direction.z;
            double t2 = (bbox.max.z - ray.origin.z) / ray.direction.z;
            tmin = std::max(tmin, std::min(t1, t2));
            tmax = std::min(tmax, std::max(t1, t2));
        }

        if (tmin <= tmax)
            return std::make_pair(tmin, tmax);
        return std::nullopt;
    }

    /// @brief Estimates the distance from a point to the Mandelbulb surface using fractal iteration.
    /// @param v The point to estimate distance from.
    /// @return The estimated distance to the surface.
    double estimateDist(const Vec3 &v) const {
        Vec3 p = v;
        double derivate = 1.0;
        double radial = 0.0;
        for (int i=0; i<_nbIters; i++) {
            radial = length(p);
            if (radial > _bailout)
                break;

            double theta = std::acos(p.z / radial);
            double phi = std::atan2(p.y, p.x);
            double pRotate = std::pow(radial, _power);
            theta *= _power;
            phi *= _power;

            p = pRotate * Vec3(std::sin(theta) * std::cos(phi),
                               std::sin(phi) * std::sin(theta),
                               std::cos(theta));
            p += v;

            derivate = derivate * _power * std::pow(radial, _power - 1.0) + 1.0;
        }
        return 0.5 * std::log(radial) * radial / derivate;
    }

    /// @brief Computes the gradient (normal vector) at a point on the Mandelbulb surface using finite differences.
    /// @param p The point on the surface.
    /// @return The normalized gradient vector.
    Vec3 gradient(const Vec3& p) const {
        double eps = _epsilon;
        Vec3 grad;
        grad.x = estimateDist(p + Vec3(eps, 0, 0)) - estimateDist(p - Vec3(eps, 0, 0));
        grad.y = estimateDist(p + Vec3(0, eps, 0)) - estimateDist(p - Vec3(0, eps, 0));
        grad.z = estimateDist(p + Vec3(0, 0, eps)) - estimateDist(p - Vec3(0, 0, eps));
        return grad / (2.0 * eps);
    }

    Vec3 _center;                         ///< Center position of the Mandelbulb
    double _scale;                        ///< Scale factor for the Mandelbulb
    double _power;                        ///< Fractal power parameter
    int _nbIters;                         ///< Number of iterations for distance estimation
    double _bailout;                      ///< Bailout radius for iteration
    double _epsilon;                      ///< Surface proximity threshold
    int _maxSteps;                        ///< Maximum ray marching steps
    std::shared_ptr<IMaterial> _material; ///< Material of the Mandelbulb
};

extern "C" IPrimitive *create() { return new Mandelbulb(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
