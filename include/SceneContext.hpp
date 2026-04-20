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

struct SceneContext {
    Scene scene;
    Camera camera;
};

#endif /* SCENECONTEXT_HPP_ */
