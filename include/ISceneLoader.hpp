/*
 * Project: Raytracer
 * File name: ISceneLoader.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for scene file parsers; loads a SceneContext from a file path using a PrimitiveFactory.
 */

#ifndef ISCENELOADER_HPP_
    #define ISCENELOADER_HPP_
    #include <string>
    #include "Factory.hpp"
    #include "SceneContext.hpp"

class ISceneLoader {
public:
    virtual SceneContext load(const std::string &filePath, PrimitiveFactory &factory) = 0;
    virtual ~ISceneLoader() = default;
};

#endif /* ISCENELOADER_HPP_ */
