/*
 * Project: Raytracer
 * File name: Scene.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene container implementation: addPrimitive and addLight definitions.
 */

#include "Scene.hpp"
#include "BVH.hpp"

Scene::Scene() : _bvhFlag(std::make_unique<std::once_flag>()) {}
Scene::~Scene() = default;
Scene::Scene(Scene &&) noexcept = default;
Scene &Scene::operator=(Scene &&) noexcept = default;

void Scene::addPrimitive(PrimitivePtr primitive) {
    _primitives.push_back(std::move(primitive));
    _bvh.reset();
}

void Scene::addLight(std::unique_ptr<ILight> light) {
    _lights.push_back(std::move(light));
}

const BVH *Scene::bvh() const {
    std::call_once(*_bvhFlag, [this]() {
        _bvh = std::make_unique<BVH>();
        _bvh->build(_primitives);
    });
    return _bvh.get();
}
