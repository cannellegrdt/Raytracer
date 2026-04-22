/*
 * Project: Raytracer
 * File name: Scene.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene container implementation: addPrimitive and addLight definitions.
 */

#include "Scene.hpp"

void Scene::addPrimitive(PrimitivePtr primitive) {
    _primitives.push_back(std::move(primitive));
}

void Scene::addLight(std::unique_ptr<ILight> light) {
    _lights.push_back(std::move(light));
}
