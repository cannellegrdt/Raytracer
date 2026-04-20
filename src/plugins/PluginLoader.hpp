/*
 * Project: Raytracer
 * File name: PluginLoader.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scans a directory for .so plugin files, loads each via DLLoader, and registers their creators into a PrimitiveFactory.
 */

#ifndef PLUGINLOADER_HPP_
    #define PLUGINLOADER_HPP_
    #include <vector>
    #include <string>
    #include "Factory.hpp"
    #include "IPrimitive.hpp"
    #include "DLLoader.hpp"

class PluginLoader {
public:
    void loadAll(const std::string &pluginsDir, PrimitiveFactory &factory);

private:
    std::vector<DLLoader<IPrimitive>> loadedPlugins_;
};

#endif /* PLUGINLOADER_HPP_ */
