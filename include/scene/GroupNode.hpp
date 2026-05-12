/*
 * Project: Raytracer
 * File name: GroupNode.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene graph group node implementation that gathers N children
 *                   primitives under a shared local transform, enabling transform
 *                   hierarchies in the scene graph.
 */

#ifndef GROUPNODE_HPP_
    #define GROUPNODE_HPP_
    #include <vector>
    #include "Type.hpp"
    #include "Mat4.hpp"

/// @brief Scene graph node that groups N children primitives under a shared local transform.
/// @details Implements IPrimitive: intersect() transforms the incoming ray to the group's local
/// space, queries every child, then transforms the closest hit (point, normal) back to world space.
/// Children may themselves be GroupNodes, enabling arbitrary-depth transform hierarchies.
class GroupNode : public IPrimitive {
public:
    GroupNode() = default;

    /// @brief Constructs a group from a list of children with no local transform.
    explicit GroupNode(std::vector<PrimitivePtr> children);

    /// @brief Constructs a group with children and a local-to-world transform.
    GroupNode(std::vector<PrimitivePtr> children, const Mat4 &localTransform);

    /// @brief Appends a child primitive to the group.
    void addChild(PrimitivePtr child);

    /// @brief Sets (or replaces) the group's local-to-world transform.
    void setTransform(const Mat4 &localTransform);

    std::optional<HitRecord> intersect(const Ray &ray) const override;

    /// @brief No-op: groups carry no intrinsic geometry.
    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override;

    /// @brief Returns the axis-aligned bounding box enclosing all child primitives, transformed by the group's transform.
    AABB boundingBox() const override;

private:
    std::vector<PrimitivePtr> _children;   ///< Child primitives contained in this group.
    Mat4 _transform;                       ///< Local-to-world transformation matrix.
    Mat4 _invTransform;                    ///< Inverse transformation matrix for normal transformation.
    bool _hasTransform;                    ///< Flag indicating if a non-identity transform is set.
};

/// @brief Creates a PrimitivePtr owning an empty GroupNode.
PrimitivePtr makeGroup();

/// @brief Creates a PrimitivePtr owning a GroupNode from a list of children.
PrimitivePtr makeGroup(std::vector<PrimitivePtr> children);

/// @brief Creates a PrimitivePtr owning a GroupNode with children and a local transform.
PrimitivePtr makeGroup(std::vector<PrimitivePtr> children, const Mat4 &localTransform);

#endif /* GROUPNODE_HPP_ */
