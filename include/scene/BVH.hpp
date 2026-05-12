/*
 * Project: Raytracer
 * File name: BVH.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Bounding Volume Hierarchy (BVH) acceleration structure implementation
 *                   for efficient O(log n) ray-scene intersection queries using the
 *                   Surface Area Heuristic (SAH) for optimal tree construction.
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
    /// @brief Node in the BVH tree.
    struct Node {
        AABB box;          ///< Bounding box of this node.
        int left{-1};      ///< Index of left child node (-1 if leaf).
        int right{-1};     ///< Index of right child node (-1 if leaf).
        int begin{0};      ///< Start index of primitives in this node.
        int end{0};        ///< End index of primitives in this node (exclusive).
    };

    std::vector<Node> _nodes;         ///< All nodes in the BVH tree.
    std::vector<int> _primIndices;    ///< Permuted indices into primitives (bounded only).
    std::vector<int> _unbounded;      ///< Indices of infinite primitives (always tested).

    /// @brief Traverses the BVH tree and finds the closest hit.
    /// @param nodeIdx Index of the current node to traverse.
    /// @param ray Ray to test for intersection.
    /// @param tMin Minimum t value for intersection (updated to entry t).
    /// @param tMax Maximum t value for intersection (updated to exit t).
    /// @param prims List of primitives in the scene.
    /// @return Optional HitRecord containing the closest intersection data.
    std::optional<HitRecord> traverseNode(int nodeIdx, const Ray &ray, double tMin, double &tMax,
        const std::vector<PrimitivePtr> &prims) const;
};

#endif /* BVH_HPP_ */
