/*
 * Project: Raytracer
 * File name: Scene.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene container class that holds all geometric primitives
 *                   and light sources for a render, with lazy BVH construction
 *                   for efficient ray-scene intersection tests.
 */


#ifndef SCENE_HPP_
    #define SCENE_HPP_
    #include <vector>
    #include <memory>
    #include <mutex>
    #include "Factory.hpp"
    #include "ILight.hpp"
    #include "Color.hpp"

class BVH;

/// @brief Container for all scene objects (primitives and lights).
class Scene {
public:
    Scene();
    ~Scene();
    Scene(Scene &&) noexcept;
    Scene &operator=(Scene &&) noexcept;

    /// @brief Adds a primitive to the scene.
    /// @param primitive Unique pointer to primitive.
    void addPrimitive(PrimitivePtr primitive);

    /// @brief Adds a light to the scene.
    /// @param light Unique pointer to light.
    void addLight(std::unique_ptr<ILight> light);

    /// @brief Gets all primitives in the scene.
    /// @return Const reference to primitive vector.
    std::vector<PrimitivePtr> &primitives() { return _primitives; }
    const std::vector<PrimitivePtr> &primitives() const { return _primitives; }

    /// @brief Gets all lights in the scene.
    /// @return Const reference to light vector.
    std::vector<std::unique_ptr<ILight>> &lights() { return _lights; }
    const std::vector<std::unique_ptr<ILight>> &lights() const { return _lights; }

    void setBackgroundColor(const Color &color) { _backgroundColor = color; }
    const Color &backgroundColor() const { return _backgroundColor; }

    /// @brief Returns a lazily-built BVH for the current primitive list.
    const BVH *bvh() const;

private:
    std::vector<PrimitivePtr> _primitives;        ///< List of geometric primitives in the scene.
    std::vector<std::unique_ptr<ILight>> _lights; ///< List of light sources in the scene.
    Color _backgroundColor;                       ///< Background color for rays that don't hit anything.
    std::unique_ptr<std::once_flag> _bvhFlag;     ///< Flag for thread-safe BVH initialization.
    mutable std::unique_ptr<BVH> _bvh;            ///< Lazily-built BVH for efficient ray-scene intersection.
};

#endif /* SCENE_HPP_ */
