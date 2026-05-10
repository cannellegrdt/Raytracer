/*
 * Project: Raytracer
 * File name: main.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Entry point of raytracer project.
 */

#include <filesystem>
#include <iostream>
#include <string>
#include <atomic>
#include <csignal>
#include "Factories.hpp"
#include "LibconfigLoader.hpp"
#include "Renderer.hpp"
#include "PluginLoader.hpp"

static std::atomic<bool> g_shouldStop{false};

static void signalHandler(int) {
    g_shouldStop.store(true, std::memory_order_relaxed);
}

static void printHelp() {
    std::cout << "USAGE: ./raytracer [OPTIONS] <scene_file>\n"
              << "\tscene_file\tpath to a .cfg scene file\n"
              << "OPTIONS:\n"
              << "\t--gui\t\tdisplay live rendering window (SFML)\n"
              << "\t--help, -h\tshow this help\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: missing scene file. Use --help for usage.\n";
        return 84;
    }

    bool gui = false;
    std::string scene;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }
        if (arg == "--gui")
            gui = true;
        else if (scene.empty())
            scene = arg;
        else {
            std::cerr << "Error: too many arguments. Use --help for usage.\n";
            return 84;
        }
    }

    if (scene.empty()) {
        std::cerr << "Error: missing scene file. Use --help for usage.\n";
        return 84;
    }

    PrimitiveFactory primitiveFact;
    LoaderFactory loaderFact;

    loaderFact.registerType(".cfg", []() {
        return std::make_unique<LibconfigLoader>();
    });

    PluginLoader pluginLoader;
    pluginLoader.loadAll("./plugins", primitiveFact);

    std::string ext = std::filesystem::path(scene).extension().string();
    auto loaderOpt = loaderFact.create(ext);
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

        if (gui)
            std::signal(SIGINT, signalHandler);

        Renderer renderer;
        renderer.render(ctx, outputPath, gui, &g_shouldStop);

        if (g_shouldStop.load(std::memory_order_relaxed)) {
            std::cout << "Interrupted. Partially rendered output written to " << outputPath << "\n";
            return 0;
        }

        std::cout << "Successfully created " << outputPath << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    return 0;
}
