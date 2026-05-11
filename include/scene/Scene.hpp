/*
 * Project: Raytracer
 * File name: Scene.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene container holding all primitives and lights for a render.
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
    std::vector<PrimitivePtr> _primitives;
    std::vector<std::unique_ptr<ILight>> _lights;
    Color _backgroundColor{0, 0, 0};
    std::unique_ptr<std::once_flag> _bvhFlag;
    mutable std::unique_ptr<BVH> _bvh;
};

#endif /* SCENE_HPP_ */
