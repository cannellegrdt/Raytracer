/*
 * Project: Raytracer
 * File name: tanglecube.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Tanglecube implicit surface plugin: x⁴-5x²+y⁴-5y²+z⁴-5z²+11.8=0, solved by bounded ray marching + bisection.
 */

#include <algorithm>
#include <cmath>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Tangle cube implicit surface plugin using ray marching with bisection refinement.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
/// Implements the implicit surface: x⁴-5x²+y⁴-5y²+z⁴-5z²+11.8=0.
class Tanglecube : public IPrimitive {
public:
    /// @brief Default constructor initializing tangle cube parameters to default values.
    Tanglecube() : _center(0, 0, 0), _scale(1.0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Tanglecube() override = default;

    /// @brief Configures the tangle cube with position, scale, and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "s" (scale, default 1.0).
    /// @param mat Shared pointer to the tangle cube material.
    /// @throws std::invalid_argument If scale is non-positive.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _scale = params.count("s") ? params.at("s") : 1.0;
        if (_scale <= 0.0)
            throw std::invalid_argument("Tanglecube scale must be > 0");
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box enclosing the tanglecube.
    AABB boundingBox() const override {
        double r = _bound * _scale;
        Vec3 rv(r, r, r);
        return AABB(_center - rv, _center + rv);
    }

    /// @brief Computes the nearest ray-tanglecube intersection via bounded ray marching and bisection.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 orig = ray.origin - _center;
        Vec3 dir = ray.direction;

        double a = dot(dir, dir);
        double h = dot(orig, dir);
        double c = dot(orig, orig) - (_bound * _scale) * (_bound * _scale);
        double disc = h * h - a * c;
        if (disc < 0.0) return std::nullopt;

        double sqrtDisc = std::sqrt(disc);
        double tNear = (-h - sqrtDisc) / a;
        double tFar  = (-h + sqrtDisc) / a;
        if (tFar < epsilon) return std::nullopt;
        if (tNear < epsilon) tNear = epsilon;

        const int steps = 256;
        double step = (tFar - tNear) / steps;
        double tCur = tNear;
        double fPrev = evalF((orig + dir * tCur) / _scale);
        double tHit = -1.0;

        for (int i = 1; i <= steps; i++) {
            double tNext = tNear + i * step;
            double fNext = evalF((orig + dir * tNext) / _scale);
            if (fPrev * fNext < 0.0) {
                double ta = tCur, tb = tNext, fa = fPrev;
                for (int j = 0; j < 64; j++) {
                    double tm = (ta + tb) * 0.5;
                    double fm = evalF((orig + dir * tm) / _scale);
                    if (fa * fm < 0.0)
                        tb = tm;
                    else {
                        ta = tm;
                        fa = fm;
                    }
                }
                tHit = (ta + tb) * 0.5;
                break;
            }
            tCur = tNext;
            fPrev = fNext;
        }

        if (tHit < epsilon) return std::nullopt;

        Vec3 localPoint = orig + dir * tHit;
        Vec3 point = localPoint + _center;
        Vec3 normal = normalize(gradient(localPoint / _scale));
        bool frontFace = dot(dir, normal) < 0.0;
        if (!frontFace) normal = -normal;

        double u = 0.5 + std::atan2(localPoint.z, localPoint.x) / (2.0 * M_PI);
        double v = 0.5 - std::asin(std::clamp(localPoint.y / (_bound * _scale), -1.0, 1.0)) / M_PI;

        Vec3 up(0, 1, 0);
        Vec3 tangent = (std::abs(normal.y) > 0.999) ? Vec3(1, 0, 0) : normalize(cross(up, normal));
        Vec3 bitangent = normalize(cross(normal, tangent));

        return HitRecord{tHit, point, normal, _material, frontFace, {u, v}, tangent, bitangent};
    }

private:
    Vec3 _center;                           ///< Center position of the tangle cube
    double _scale;                          ///< Scale factor for the tangle cube
    std::shared_ptr<IMaterial> _material;   ///< Material of the tangle cube
    
    static constexpr double _bound = 4.0;   ///< Bounding radius for initial ray intersection test

    /// @brief Evaluates the tangle cube implicit function at point p.
    /// @param p Point to evaluate (in object space, pre-scaled).
    /// @return Value of x⁴-5x²+y⁴-5y²+z⁴-5z²+11.8.
    double evalF(const Vec3 &p) const {
        double x2 = p.x * p.x, y2 = p.y * p.y, z2 = p.z * p.z;
        return x2*x2 - 5.0*x2 + y2*y2 - 5.0*y2 + z2*z2 - 5.0*z2 + 11.8;
    }

    /// @brief Computes the gradient (surface normal direction) at point p.
    /// @param p Point to evaluate (in object space, pre-scaled).
    /// @return Gradient vector of the implicit function.
    Vec3 gradient(const Vec3 &p) const {
        return {
            4.0*p.x*p.x*p.x - 10.0*p.x,
            4.0*p.y*p.y*p.y - 10.0*p.y,
            4.0*p.z*p.z*p.z - 10.0*p.z
        };
    }
};

extern "C" IPrimitive *create() { return new Tanglecube(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
