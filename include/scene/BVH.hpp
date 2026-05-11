/*
 * Project: Raytracer
 * File name: BVH.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Bounding Volume Hierarchy for O(log n) ray-scene intersection queries.
 */

#ifndef BVH_HPP_
    #define BVH_HPP_
    #include <vector>
    #include <optional>
    #include "AABB.hpp"
    #include "HitRecord.hpp"
    #include "Type.hpp"

/// @brief Bounding Volume Hierarchy acceleration structure.
class BVH {
public:
    /// @brief Builds the BVH from a set of primitives.
    /// @param primitives Scene primitive list.
    void build(const std::vector<PrimitivePtr> &primitives);

    /// @brief Traverses the BVH and returns the closest hit.
    /// @param ray        Ray to test.
    /// @param primitives Same primitive list that was used for build().
    /// @return Closest HitRecord, or nullopt if no intersection.
    std::optional<HitRecord> intersect(const Ray &ray, const std::vector<PrimitivePtr> &primitives) const;

private:
    struct Node {
        AABB box;
        int left{-1};
        int right{-1};
        int begin{0};
        int end{0};
    };

    std::vector<Node> _nodes;
    std::vector<int> _primIndices; ///< Permuted indices into primitives (bounded only).
    std::vector<int> _unbounded;   ///< Indices of infinite primitives (always tested).

    std::optional<HitRecord> traverseNode(int nodeIdx, const Ray &ray, double tMin, double &tMax,
        const std::vector<PrimitivePtr> &prims) const;
};

#endif /* BVH_HPP_ */
