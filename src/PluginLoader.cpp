/*
 * Project: Raytracer
 * File name: PluginLoader.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of PluginLoader::loadAll; iterates .so files in a directory and registers each plugin's creator/destroyer into the factory.
 */

#include <iostream>
#include <filesystem>
#include "PluginLoader.hpp"

namespace fs = std::filesystem;

void PluginLoader::loadAll(const std::string &pluginsDir, PrimitiveFactory &factory) {
    try {
        if (fs::exists(pluginsDir) && fs::is_directory(pluginsDir)) {
            for (const auto &entry : fs::directory_iterator(pluginsDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".so") {
                    std::string path = entry.path().string();
                    std::string primitiveName = entry.path().stem().string();

                    try {
                        DLLoader<IPrimitive> loader(path);
                        auto rawCreate  = loader.getCreate();
                        auto rawDestroy = loader.getDestroy();
                        factory.registerType(primitiveName, [rawCreate, rawDestroy]() -> PrimitivePtr {
                            return PrimitivePtr(rawCreate(), rawDestroy);
                        });
                        _loadedPlugins.push_back(std::move(loader));
                    } catch (const std::exception &e) {
                        std::cerr << "Error: failed to load plugin " << path << ": " << e.what() << std::endl;
                    }
                }
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
