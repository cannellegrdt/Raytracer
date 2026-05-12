/*
 * Project: Raytracer
 * File name: LibconfigLoader.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: ISceneLoader implementation that parses .cfg scene files using libconfig++
 *                   for loading scene configurations including primitives, materials, lights,
 *                   and camera settings.
 */

#ifndef LIBCONFIGLOADER_HPP_
    #define LIBCONFIGLOADER_HPP_
    #include <string>
    #include <set>
    #include "ISceneLoader.hpp"

/// @brief Loader for .cfg scene files using libconfig++.
/// @details Implements ISceneLoader to parse libconfig format files and
/// construct a SceneContext with primitives and camera.
class LibconfigLoader : public ISceneLoader {
public:
    /// @brief Loads a scene from a libconfig .cfg file.
    /// @param filePath Path to the .cfg scene file.
    /// @param factory Factory for creating primitives.
    /// @return Complete scene context with primitives and camera.
    SceneContext load(const std::string &filePath, PrimitiveFactory &factory) override;

private:
    /// @brief Internal load with visited set.
    /// @param filePath Path to the .cfg scene file.
    /// @param factory Factory for creating primitives.
    /// @param visited Absolute paths already visited.
    /// @return Complete scene context with primitives and camera.
    SceneContext load(const std::string &filePath, PrimitiveFactory &factory, std::set<std::string> visited);
};

#endif /* LIBCONFIGLOADER_HPP_ */
