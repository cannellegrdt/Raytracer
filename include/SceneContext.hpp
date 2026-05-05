/*
 * Project: Raytracer
 * File name: SceneContext.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Aggregate of a Scene and a Camera representing a fully-described render setup.
 */

#ifndef SCENECONTEXT_HPP_
    #define SCENECONTEXT_HPP_
    #include <optional>
    #include "Scene.hpp"
    #include "Camera.hpp"

struct Supersampling {
    int nbSamples;
};

/// @brief Complete render context containing scene and camera.
struct SceneContext {
    Scene scene;                               ///< Scene with all primitives and lights.
    std::optional<Camera> camera;              ///< Camera for rendering, optional to allow inheritance from imports.
    std::optional<Supersampling> antialiasing; ///< Supersampling to enable antialiasing.
};

#endif /* SCENECONTEXT_HPP_ */
