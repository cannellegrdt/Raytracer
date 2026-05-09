/*
 * Project: Raytracer
 * File name: AABB.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Axis-Aligned Bounding Box with slab-method ray intersection test and merge utility.
 */

#ifndef AABB_HPP_
    #define AABB_HPP_
    #include <algorithm>
    #include <limits>
    #include "Vec3.hpp"
    #include "Ray.hpp"

/// @brief Axis-Aligned Bounding Box.
struct AABB {
    Vec3 min{0, 0, 0};
    Vec3 max{0, 0, 0};

    AABB() = default;
    AABB(const Vec3 &mn, const Vec3 &mx) : min(mn), max(mx) {}

    /// @brief Returns a box that contains nothing (identity for merge).
    static AABB empty() {
        constexpr double inf = std::numeric_limits<double>::infinity();
        return AABB({inf, inf, inf}, {-inf, -inf, -inf});
    }

    /// @brief Returns a box that spans all space.
    static AABB infinite() {
        constexpr double inf = std::numeric_limits<double>::infinity();
        return AABB({-inf, -inf, -inf}, {inf, inf, inf});
    }

    /// @brief Returns true if any extent of this box is infinite.
    bool isInfinite() const {
        constexpr double inf = std::numeric_limits<double>::infinity();
        return min.x == -inf || max.x == inf ||
               min.y == -inf || max.y == inf ||
               min.z == -inf || max.z == inf;
    }

    /// @brief Returns the center of the box.
    Vec3 centroid() const {
        return (min + max) * 0.5;
    }

    /// @brief Returns the smallest AABB containing both a and b.
    static AABB merge(const AABB &a, const AABB &b) {
        return AABB(
            Vec3(std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)),
            Vec3(std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z))
        );
    }

    /// @brief Slab-method ray-AABB intersection test.
    /// @param ray  Ray to test.
    /// @param tMin Near bound (updated to entry t, clamped to 0 for inside-box rays).
    /// @param tMax Far bound (updated to exit t).
    /// @return True if the ray intersects the box within [tMin, tMax].
    bool intersect(const Ray &ray, double &tMin, double &tMax) const {
        double invDx = 1.0 / ray.direction.x;
        double tx0 = (min.x - ray.origin.x) * invDx;
        double tx1 = (max.x - ray.origin.x) * invDx;

        if (invDx < 0.0)
            std::swap(tx0, tx1);
        if (tx0 > tMin)
            tMin = tx0;
        if (tx1 < tMax)
            tMax = tx1;
        if (tMax <= tMin)
            return false;

        double invDy = 1.0 / ray.direction.y;
        double ty0 = (min.y - ray.origin.y) * invDy;
        double ty1 = (max.y - ray.origin.y) * invDy;

        if (invDy < 0.0)
            std::swap(ty0, ty1);
        if (ty0 > tMin)
            tMin = ty0;
        if (ty1 < tMax)
            tMax = ty1;
        if (tMax <= tMin)
            return false;

        double invDz = 1.0 / ray.direction.z;
        double tz0 = (min.z - ray.origin.z) * invDz;
        double tz1 = (max.z - ray.origin.z) * invDz;

        if (invDz < 0.0)
            std::swap(tz0, tz1);
        if (tz0 > tMin)
            tMin = tz0;
        if (tz1 < tMax)
            tMax = tz1;
        if (tMax <= tMin)
            return false;

        tMin = std::max(tMin, 0.0);
        return true;
    }
};

#endif /* AABB_HPP_ */
