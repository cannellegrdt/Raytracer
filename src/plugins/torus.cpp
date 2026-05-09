/*
 * Project: Raytracer
 * File name: torus.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Torus primitive plugin implementing ray-torus intersection via quartic equation solving.
 */

#include <algorithm>
#include <cmath>
#include <vector>
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"

/// @brief Torus primitive implementing ray-torus intersection via quartic equation solving.
/// Inherits from IPrimitive to integrate with the raytracer plugin system.
class Torus : public IPrimitive {
public:
    /// @brief Default constructor initializing torus parameters to default values.
    Torus() : _center(0, 0, 0), _majorRadius(0.0), _minorRadius(0.0), _axis(0, 0, 0), _material(nullptr) {}
    /// @brief Default destructor overriding IPrimitive.
    ~Torus() override = default;

    /// @brief Configures the torus with parameters and material.
    /// @param params Unordered map of parameters: "x", "y", "z" (center), "R" (major radius), "r" (minor radius), "ax", "ay", "az" (axis).
    /// @param mat Shared pointer to the torus material.
    /// @throws std::invalid_argument If radii are non-positive, axis is zero, or minor > major (swaps them).
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override {
        _center = { params.at("x"), params.at("y"), params.at("z") };
        _majorRadius = params.at("R");
        _minorRadius = params.at("r");
        if (_minorRadius <= 0.0 || _majorRadius <= 0.0)
            throw std::invalid_argument("Torus radius must be > 0");
        if (_minorRadius > _majorRadius) {
            double tempRadius = _majorRadius;
            _majorRadius = _minorRadius;
            _minorRadius = tempRadius;
        }
        _axis = { params.at("ax"), params.at("ay"), params.at("az") };
        if (length(_axis) <= epsilon)
            throw std::invalid_argument("Torus axis must be > 0");
        _material = std::move(mat);
    }

    /// @brief Returns the axis-aligned bounding box enclosing the torus.
    AABB boundingBox() const override {
        double r = _majorRadius + _minorRadius;
        Vec3 rv(r, r, r);
        return AABB(_center - rv, _center + rv);
    }

    /// @brief Computes the nearest ray-torus intersection.
    /// @param ray The ray to test for intersection.
    /// @return Optional HitRecord with intersection details, or std::nullopt if no hit.
    std::optional<HitRecord> intersect(const Ray &ray) const override {
        Vec3 axis = normalize(_axis);
        Vec3 tangent = normalize(cross(axis, std::abs(axis.x) < std::abs(axis.y) ? Vec3(1,0,0) : Vec3(0,1,0)));
        Vec3 bitangent = cross(axis, tangent);

        Vec3 localOrigin = ray.origin - _center;
        Vec3 localDir = ray.direction;

        Vec3 transOrigin = {dot(localOrigin, tangent), dot(localOrigin, bitangent), dot(localOrigin, axis)};
        Vec3 transDir = {dot(localDir, tangent), dot(localDir, bitangent), dot(localDir, axis)};

        double m = dot(transDir, transDir);
        double n = dot(transOrigin, transDir);
        double p = dot(transOrigin, transOrigin);
        double k = p + _majorRadius * _majorRadius - _minorRadius * _minorRadius;

        double a = transDir.x * transDir.x + transDir.y * transDir.y;
        double b = transOrigin.x * transDir.x + transOrigin.y * transDir.y;
        double c = transOrigin.x * transOrigin.x + transOrigin.y * transOrigin.y;

        double A = m * m;
        double B = 4.0 * m * n;
        double C = 4.0 * n * n + 2.0 * m * k - 4.0 * _majorRadius * _majorRadius * a;
        double D = 4.0 * n * k - 8.0 * _majorRadius * _majorRadius * b;
        double E = k * k - 4.0 * _majorRadius * _majorRadius * c;

        auto roots = solveQuartic(A, B, C, D, E);

        if (roots.empty()) return std::nullopt;

        double t = roots[0];
        Vec3 localPoint = transOrigin + transDir * t;
        Vec3 point = _center + tangent * localPoint.x + bitangent * localPoint.y + axis * localPoint.z;

        double s = localPoint.x * localPoint.x + localPoint.y * localPoint.y + localPoint.z * localPoint.z + _majorRadius * _majorRadius - _minorRadius * _minorRadius;
        Vec3 localNormal = normalize({
            4.0 * localPoint.x * s - 8.0 * _majorRadius * _majorRadius * localPoint.x,
            4.0 * localPoint.y * s - 8.0 * _majorRadius * _majorRadius * localPoint.y,
            4.0 * localPoint.z * s
        });
        Vec3 normal = normalize(tangent * localNormal.x + bitangent * localNormal.y + axis * localNormal.z);

        bool frontFace = dot(ray.direction, normal) < 0;
        if (!frontFace)
            normal = -normal;

        double u = std::atan2(localPoint.y, localPoint.x) / (2.0 * M_PI);
        if (u < 0.0)
            u += 1.0;
        double v = std::atan2(localPoint.z, std::sqrt(localPoint.x * localPoint.x + localPoint.y * localPoint.y) - _majorRadius) / (2.0 * M_PI);
        if (v < 0.0)
            v += 1.0;

        Vec3 surfTangent = normalize(bitangent * (-std::sin(2.0 * M_PI * u)) + tangent * std::cos(2.0 * M_PI * u));
        Vec3 surfBitangent = normalize(cross(normal, surfTangent));

        return HitRecord{t, point, normal, _material, frontFace, {u, v}, surfTangent, surfBitangent};
    };

private:
    /// @brief Solves quartic equation a*x⁴ + b*x³ + c*x² + d*x + e = 0 for positive roots.
    /// @param a Coefficient of x⁴ (must be non-zero).
    /// @param b Coefficient of x³.
    /// @param c Coefficient of x².
    /// @param d Coefficient of x.
    /// @param e Constant term.
    /// @return Sorted vector of positive real roots (t > epsilon), empty if none.
    std::vector<double> solveQuartic(double a, double b, double c, double d, double e) const {
        std::vector<double> roots;

        if (std::abs(a) < epsilon)
            return roots;

        b /= a;
        c /= a;
        d /= a;
        e /= a;
        a = 1.0;

        double shift = b / 4.0;
        double p = c - 3.0 * b * b / 8.0;
        double q = b * b * b / 8.0 - b * c / 2.0 + d;
        double r = -3.0 * b * b * b * b / 256.0 + b * b * c / 16.0 - b * d / 4.0 + e;

        double b3 = -p / 2.0;
        double c3 = -r;
        double d3 = (4.0 * p * r - q * q) / 8.0;
        double shift3 = b3 / 3.0;
        double p3 = c3 - b3 * b3 / 3.0;
        double q3 = 2.0 * b3 * b3 * b3 / 27.0 - b3 * c3 / 3.0 + d3;
        double disc3 = q3 * q3 / 4.0 + p3 * p3 * p3 / 27.0;

        double y = 0.0;
        if (disc3 > epsilon) {
            double sqrt_disc = std::sqrt(disc3);
            double u = std::cbrt(-q3 / 2.0 + sqrt_disc);
            double v = std::cbrt(-q3 / 2.0 - sqrt_disc);
            y = u + v - shift3;
        } else {
            double rho = std::sqrt(-p3 * p3 * p3 / 27.0);
            double theta = std::acos(-q3 / (2.0 * rho));
            y = 2.0 * std::cbrt(rho) * std::cos(theta / 3.0) - shift3;
        }

        double alphaFirst = 2.0 * y - p;
        if (alphaFirst < epsilon)
            return roots;
        double alpha = std::sqrt(alphaFirst);
        double beta = (y - q / (2.0 * alpha));
        double gamma = (y + q / (2.0 * alpha));

        double disc1 = alpha * alpha - 4.0 * beta;
        if (disc1 >= 0.0) {
            double sqrt_disc1 = sqrt(disc1);
            roots.push_back(-alpha - sqrt_disc1 - shift);
            roots.push_back(-alpha + sqrt_disc1 - shift);
        } 
        double disc2 = alpha * alpha - 4.0 * gamma;
        if (disc2 >= 0.0) {
            double sqrt_disc2 = sqrt(disc2);
            roots.push_back(alpha - sqrt_disc2 - shift);
            roots.push_back(alpha + sqrt_disc2 - shift);
        }

        std::vector<double> valid_roots;
        for (double t : roots) {
            if (t > epsilon)
                valid_roots.push_back(t);
        }
        std::sort(valid_roots.begin(), valid_roots.end());
        
        return valid_roots;
    }

    Vec3 _center;                         ///< Center position of the torus
    double _majorRadius;                  ///< Major radius (distance from center to tube center)
    double _minorRadius;                  ///< Minor radius (tube radius)
    Vec3 _axis;                           ///< Axis of revolution for the torus
    std::shared_ptr<IMaterial> _material; ///< Material of the torus
};

extern "C" IPrimitive *create() { return new Torus(); };
extern "C" void destroy(IPrimitive *p) { delete p; };
