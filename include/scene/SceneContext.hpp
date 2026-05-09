/*
 * Project: Raytracer
 * File name: SceneContext.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Aggregate of a Scene and a Camera representing a fully-described render setup.
 */

#ifndef SCENECONTEXT_HPP_
    #define SCENECONTEXT_HPP_
    #include <optional>
    #include <string>
    #include "Scene.hpp"
    #include "Camera.hpp"

struct Supersampling {
    int samples;      ///< Number of samples per dimension. Must be >0.
    std::string type; ///< Supersampling type: "uniform" or "adaptive". Default is "uniform".
    double threshold; ///< Edge detection threshold for adaptive mode (0.0 to 1.0). Only used when type is "adaptative"
};

/// @brief Complete render context containing scene and camera.
struct SceneContext {
    Scene scene;                               ///< Scene with all primitives and lights.
    std::optional<Camera> camera;              ///< Camera for rendering, optional to allow inheritance from imports.
    std::optional<Supersampling> antialiasing; ///< Supersampling to enable antialiasing.
    std::optional<int> nbAORays;               ///< Number of ambient occlusion rays (default: 16).
};

#endif /* SCENECONTEXT_HPP_ */
