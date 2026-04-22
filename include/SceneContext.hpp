/*
 * Project: Raytracer
 * File name: SceneContext.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Aggregate of a Scene and a Camera representing a fully-described render setup.
 */

#ifndef SCENECONTEXT_HPP_
    #define SCENECONTEXT_HPP_
    #include "Scene.hpp"
    #include "Camera.hpp"

/// @brief Complete render context containing scene and camera.
struct SceneContext {
    Scene scene;   ///< Scene with all primitives and lights.
    Camera camera; ///< Camera for rendering.
    std::string filename; ///< Input file name (without extension) for output naming.
};

#endif /* SCENECONTEXT_HPP_ */
