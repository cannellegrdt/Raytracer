/*
 * Project: Raytracer
 * File name: LibconfigLoader.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene loading implementation; reads camera, primitives and lights from a libconfig++ .cfg file into a SceneContext.
 */

#include <string>
#include <iostream>
#include <libconfig.h++>
#include <optional>
#include <functional>
#include <vector>
#include <set>
#include <filesystem>
#include <unordered_map>
#include "LibconfigLoader.hpp"
#include "PrimitiveBuilder.hpp"
#include "ConfigUtils.hpp"
#include "MaterialBuilder.hpp"
#include "TransformBuilder.hpp"
#include "PrimitivesParser.hpp"
#include "LightsParser.hpp"
#include "CameraParser.hpp"
#include "RendererParser.hpp"

SceneContext LibconfigLoader::load(const std::string &filePath, PrimitiveFactory &factory) {
    return load(filePath, factory, {});
}

SceneContext LibconfigLoader::load(const std::string &filePath, PrimitiveFactory &factory, std::set<std::string> visited) {
    std::string absPath = std::filesystem::canonical(filePath).string();
    if (visited.count(absPath))
        throw std::runtime_error("Circular import detected: " + absPath);
    visited.insert(absPath);

    libconfig::Config cfg;

    try {
        cfg.readFile(filePath.c_str());
    } catch (const libconfig::FileIOException &e) {
        throw std::runtime_error("Cannot read file: " + filePath);
    } catch (const libconfig::ParseException &e) {
        throw std::runtime_error(std::string("Parse error in ") + e.getFile() +
            " at line " + std::to_string(e.getLine()) + ": " + e.getError());
    }

    for (const auto &key : {"primitives", "lights"}) {
        if (!ConfigUtils::checkValues(cfg, key))
            throw std::runtime_error(std::string("Missing key: ") + key);
    }

    std::optional<Camera> cameraOpt = CameraParser::parse(cfg);
    std::optional<Supersampling> antialiasingOpt = RendererParser::parseAntialiasing(cfg);
    std::optional<int> nbAORaysOpt = RendererParser::parseAORays(cfg);

    Scene scene;
    PrimitiveBuilder builder(factory);

    auto prims = PrimitivesParser::parse(cfg.lookup("primitives"), builder);
    for (auto &p : prims)
        scene.addPrimitive(std::move(p));

    LightsParser::parseAndAddLights(cfg.lookup("lights"), scene);

    if (cfg.exists("imports")) {
        const libconfig::Setting &imports = cfg.lookup("imports");
        for (int i = 0; i < imports.getLength(); i++) {
            const libconfig::Setting &imp = imports[i];
            std::string subFilePath = imp.lookup("path").c_str();

            SceneContext importedCtx = load(subFilePath, factory, visited);

            if (!cameraOpt && importedCtx.camera)
                cameraOpt = importedCtx.camera;

            if (!antialiasingOpt && importedCtx.antialiasing)
                antialiasingOpt = importedCtx.antialiasing;

            if (!nbAORaysOpt && importedCtx.nbAORays)
                nbAORaysOpt = importedCtx.nbAORays;

            auto &importedPrims = importedCtx.scene.primitives();
            for (auto &prim : importedPrims)
                scene.addPrimitive(TransformBuilder::applyTransforms(std::move(prim), imp));
            importedPrims.clear();

            auto &importedLights = importedCtx.scene.lights();
            for (auto &light : importedLights)
                scene.addLight(std::move(light));
            importedLights.clear();
        }
    }

    if (cfg.exists("background")) {
        const libconfig::Setting &bg = cfg.lookup("background");
        scene.setBackgroundColor(Color(
            ConfigUtils::toDouble(bg["r"]),
            ConfigUtils::toDouble(bg["g"]),
            ConfigUtils::toDouble(bg["b"])
        ));
    }

    if (!cameraOpt)
        throw std::runtime_error("No camera found in the configuration files");
    return SceneContext{std::move(scene), cameraOpt, antialiasingOpt, nbAORaysOpt};
}
