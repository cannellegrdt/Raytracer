/*
 * Project: Raytracer
 * File name: IPrimitive.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for geometric primitives; defines ray-intersection returning an optional HitRecord.
 */

#ifndef IPRIMITIVE_HPP_
    #define IPRIMITIVE_HPP_
    #include <memory>
    #include <optional>
    #include <stdexcept>
    #include <unordered_map>
    #include <string>
    #include "HitRecord.hpp"
    #include "Ray.hpp"
    #include "AABB.hpp"

class IMaterial;

/// @brief Interface for geometric primitives.
/// @details Implementations define how rays intersect with shapes (sphere, plane, etc.).
class IPrimitive {
public:
    /// @brief Tests intersection between ray and primitive.
    /// @param ray Ray to test against.
    /// @return Optional HitRecord if ray intersects, empty otherwise.
    virtual std::optional<HitRecord> intersect(const Ray &ray) const = 0;

    /// @brief Configures the primitive with parameters from scene file.
    /// @param params Map of parameter names to values.
    /// @param mat Shared pointer to material to apply.
    virtual void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) = 0;

    /// @brief Sets the source file path for file-based primitives (e.g. OBJ meshes).
    /// @param path Absolute or relative path to the source file.
    virtual void setFilePath(const std::string &) {}

    /// @brief Returns an axis-aligned bounding box enclosing the primitive.
    /// @return AABB::infinite() by default for unbounded primitives.
    virtual AABB boundingBox() const { return AABB::infinite(); }

    virtual ~IPrimitive() = default;
};

#endif /* IPRIMITIVE_HPP_ */
