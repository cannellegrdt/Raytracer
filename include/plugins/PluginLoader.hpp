/*
 * Project: Raytracer
 * File name: PluginLoader.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scans a directory for .so plugin files, loads each via DLLoader,
 *                   and registers their creators into a PrimitiveFactory.
 */

#ifndef PLUGINLOADER_HPP_
    #define PLUGINLOADER_HPP_
    #include <vector>
    #include <string>
    #include "Factory.hpp"
    #include "IPrimitive.hpp"
    #include "DLLoader.hpp"

/// @brief Loads and manages primitive plugins from shared libraries.
class PluginLoader {
public:
    /// @brief Loads all .so plugins from a directory.
    /// @param pluginsDir Path to directory containing .so files.
    /// @param factory Factory to register loaded primitives into.
    void loadAll(const std::string &pluginsDir, PrimitiveFactory &factory);

private:
    std::vector<DLLoader<IPrimitive>> _loadedPlugins;  ///< List of loaded plugin dynamic library wrappers.
};

#endif /* PLUGINLOADER_HPP_ */
