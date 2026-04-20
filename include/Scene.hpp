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

class Scene {
public:
    void addPrimitive(PrimitivePtr primitive);
    void addLight(std::unique_ptr<ILight> light);

    const std::vector<PrimitivePtr> &primitives() const { return primitives_; }
    const std::vector<std::unique_ptr<ILight>> &lights() const { return lights_; }

private:
    std::vector<PrimitivePtr> primitives_;
    std::vector<std::unique_ptr<ILight>> lights_;
};

#endif /* SCENE_HPP_ */
