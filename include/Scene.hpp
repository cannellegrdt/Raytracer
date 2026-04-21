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
    #include "Factory.hpp"
    #include "ILight.hpp"

/// @brief Container for all scene objects (primitives and lights).
class Scene {
public:
    /// @brief Adds a primitive to the scene.
    /// @param primitive Unique pointer to primitive.
    void addPrimitive(PrimitivePtr primitive);

    /// @brief Adds a light to the scene.
    /// @param light Unique pointer to light.
    void addLight(std::unique_ptr<ILight> light);

    /// @brief Gets all primitives in the scene.
    /// @return Const reference to primitive vector.
    const std::vector<PrimitivePtr> &primitives() const { return primitives_; }

    /// @brief Gets all lights in the scene.
    /// @return Const reference to light vector.
    const std::vector<std::unique_ptr<ILight>> &lights() const { return lights_; }

private:
    std::vector<PrimitivePtr> primitives_;
    std::vector<std::unique_ptr<ILight>> lights_;
};

#endif /* SCENE_HPP_ */
