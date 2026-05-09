/*
 * Project: Raytracer
 * File name: mobius.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Mobius strip primitive plugin implementing ray-Mobius intersection via Newton-Raphson.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Mobius strip primitive implementing ray-Mobius intersection via Newton-Raphson.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class MobiusStrip : public IPrimitive {
public:
    /// @brief Default constructor initializing Mobius parameters to default values.
    MobiusStrip() : _center(0, 0, 0), _R(1.0), _w(0.1), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~MobiusStrip() override = default;

    /// @brief Configures the Mobius strip with parameters and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "R" (major radius), "w" (half-width).
    /// @param mat Shared pointer to the Mobius material.
    /// @throws std::invalid_argument If R <= 0 or w <= 0.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _R = params.at("R");
        _w = params.at("w");
        if (_R <= 0.0 || _w <= 0.0)
            throw std::invalid_argument("Mobius R and w must be > 0");
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box enclosing the Mobius strip (approximated).
    AABB boundingBox() const override {
        double r = _R + _w;
        Vec3 rv(r, r, _w);
        return AABB(_center - rv, _center + rv);
    }

    /// @brief Computes the nearest ray-Mobius intersection using Newton-Raphson.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 transOrigin = ray.origin - _center;
        Vec3 transDir = ray.direction;

        const int numTries = 8;
        std::array<NewtonResult, numTries> candidates;
        int numCandidates = 0;

        for (int i = 0; i < numTries; i++) {
            double u0 = i * (2.0 * M_PI / numTries);
            auto result = newtonRaphson(transOrigin, transDir, u0);
            if (result.has_value() && result->t > epsilon)
                candidates[numCandidates++] = *result;
        }

        if (numCandidates == 0) return std::nullopt;

        auto minIt = std::min_element(candidates.begin(), candidates.begin() + numCandidates,
            [](const NewtonResult &a, const NewtonResult &b) { return a.t < b.t; });

        double t = minIt->t;
        double v = minIt->v;

        double u = std::fmod(minIt->u, 2.0 * M_PI);
        if (u < 0)
            u += 2.0 * M_PI;

        Vec3 point = _center + transOrigin + transDir * t;
        Vec3 normal = normalize(cross(minIt->dsu, minIt->dsv));
        if (dot(normal, transDir) > 0.0)
            normal = -normal;

        double uu = u / (2.0 * M_PI);
        double vv = (v + _w) / (2.0 * _w);

        Vec3 up = (std::abs(normal.z) < 0.9) ? Vec3(0, 0, 1) : Vec3(1, 0, 0);
        Vec3 surfTangent = normalize(cross(normal, up));
        Vec3 surfBitangent = normalize(cross(normal, surfTangent));

        return HitRecord{t, point, normal, _material, true, {uu, vv}, surfTangent, surfBitangent};
    }

private:
    struct NewtonResult {
        double t, u, v;
        Vec3 dsu, dsv;
    };

    /// @brief Evaluates the surface point and both partial derivatives in a single pass,
    /// sharing the 4 trig values (cos/sin of u and u/2) across all three computations.
    struct SurfaceEval {
        Vec3 surf, dsu, dsv;
    };
    SurfaceEval evalSurface(double u, double v) const {
        double cos_u = cos(u), sin_u = sin(u);
        double cos_u2 = cos(u * 0.5), sin_u2 = sin(u * 0.5);
        double Rv = _R + v * cos_u2;
        double vsh = v * sin_u2 * 0.5;

        return {
            {
                Rv * cos_u,
                Rv * sin_u,
                v * sin_u2
            },
            {
                -Rv * sin_u - vsh * cos_u,
                Rv * cos_u - vsh * sin_u,
                v * cos_u2 * 0.5
            },
            {
                cos_u2 * cos_u,
                cos_u2 * sin_u,
                sin_u2
            }
        };
    }

    /// @brief Newton-Raphson solver for t, u, v where ray intersects Mobius surface.
    std::optional<NewtonResult> newtonRaphson(Vec3 origin, Vec3 dir, double u0) const {
        const int maxIter = 15;
        const double tol = 1e-6;
        const double tol2 = tol * tol;
        double t = 0.0;
        double u = u0;
        double v = 0.0;

        for (int iter = 0; iter < maxIter; iter++) {
            auto ev = evalSurface(u, v);

            Vec3 F = origin + t * dir - ev.surf;
            if (dot(F, F) < tol2)
                return NewtonResult{t, u, v, ev.dsu, ev.dsv};

            Vec3 J0 = dir;
            Vec3 J1 = -ev.dsu;
            Vec3 J2 = -ev.dsv;

            double det = dot(J0, cross(J1, J2));
            if (std::abs(det) < 1e-12) break;

            Vec3 invJ0 = cross(J1, J2) / det;
            Vec3 invJ1 = cross(J2, J0) / det;
            Vec3 invJ2 = cross(J0, J1) / det;

            double dt = -dot(invJ0, F);
            double du_step = -dot(invJ1, F);
            double dv_step = -dot(invJ2, F);

            t += dt;
            u += du_step;
            v = std::clamp(v + dv_step, -_w, _w);

            if (std::abs(dt) < tol && std::abs(du_step) < tol && std::abs(dv_step) < tol)
                return NewtonResult{t, u, v, ev.dsu, ev.dsv};
        }
        return std::nullopt;
    }

    Vec3 _center;                         ///< Center position of the Mobius strip
    double _R;                            ///< Major radius
    double _w;                            ///< Half-width
    std::shared_ptr<IMaterial> _material; ///< Material of the Mobius strip

    static constexpr double epsilon = 1e-8;
};

extern "C" IPrimitive *create() { return new MobiusStrip(); }
extern "C" void destroy(IPrimitive *p) { delete p; }
