/*
 * Project: Raytracer
 * File name: main.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Entry point of raytracer project.
 */

#include <filesystem>
#include <iostream>
#include "Factories.hpp"
#include "LibconfigLoader.hpp"
#include "Renderer.hpp"
#include "PluginLoader.hpp"

/// @brief Prints usage help to standard output.
static void printHelp() {
    std::cout << "USAGE: ./raytracer <scene_file>\n"
              << "\tscene_file\tpath to a .cfg scene file\n";
}

/// @brief Main entry point of the raytracer.
/// @param argc Number of command-line arguments.
/// @param argv Array of command-line argument strings.
/// @return 0 on success, 84 on error.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: missing scene file. Use --help for usage.\n";
        return 84;
    }
    if (argc != 2) {
        std::cerr << "Error: too many arguments. Expected exactly one scene file.\n";
        return 84;
    }
    std::string scene = argv[1];
    if (scene == "--help" || scene == "-h") {
        printHelp();
        return 0;
    }

    PrimitiveFactory primitiveFact;
    LoaderFactory loaderFact;

    loaderFact.registerType(".cfg", []() {
        return std::make_unique<LibconfigLoader>();
    });

    PluginLoader pluginLoader;
    pluginLoader.loadAll("./plugins", primitiveFact);

    std::string ext = std::filesystem::path(scene).extension().string();
    auto loaderOpt  = loaderFact.create(ext);
    if (!loaderOpt) {
        std::cerr << "Error: unsupported file format '" << ext << "'.\n";
        return 84;
    }

    try {
        SceneContext ctx = (*loaderOpt)->load(scene, primitiveFact);

        std::filesystem::path inputPath(scene);
        std::string stem = inputPath.stem().string();
        std::filesystem::path outputDir = "output";
        std::filesystem::create_directories(outputDir);
        std::string outputPath = (outputDir / (stem + ".ppm")).string();

        Renderer renderer;
        renderer.render(ctx, outputPath);

        std::cout << "Successfully created " << outputPath << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    return 0;
}
