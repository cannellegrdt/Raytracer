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

/// @brief Interface for scene file loaders.
/// @details Implementations parse different file formats (JSON, XML, etc.).
class ISceneLoader {
public:
    /// @brief Loads a scene from a file.
    /// @param filePath Path to scene file.
    /// @param factory Factory for creating primitives.
    /// @return Complete scene context with primitives and camera.
    virtual SceneContext load(const std::string &filePath, PrimitiveFactory &factory) = 0;
    virtual ~ISceneLoader() = default;
};

#endif /* ISCENELOADER_HPP_ */
