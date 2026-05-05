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
#include "FlatColor.hpp"
#include "Transparency.hpp"
#include "Reflection.hpp"
#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "LibconfigLoader.hpp"
#include "PrimitiveBuilder.hpp"
#include "Decorators.hpp"
#include "Renderer.hpp"

static double toDouble(const libconfig::Setting &s) {
    if (s.getType() == libconfig::Setting::TypeInt)
        return static_cast<double>((int)s);
    if (s.getType() == libconfig::Setting::TypeInt64)
        return static_cast<double>((long long)s);
    if (s.getType() == libconfig::Setting::TypeFloat)
        return static_cast<double>((double)s);
    throw std::runtime_error(std::string("Expected numeric setting at: ") + s.getPath());
}

static std::optional<std::reference_wrapper<const libconfig::Setting>> checkValues(const libconfig::Config& cfg, const std::string& path) {
    if (cfg.exists(path))
        return std::ref(cfg.lookup(path));
    return std::nullopt;
}

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

    std::vector<std::string> requiredKeys = {
        "primitives",
        "lights"
    };
    
    for (const auto &key : requiredKeys) {
        auto setting = checkValues(cfg, key);
        if (!setting)
            throw std::runtime_error("Missing key: " + key);
    }

    std::optional<Camera> cameraOpt;
    if (cfg.exists("camera")) {
        const libconfig::Setting &cam = cfg.lookup("camera");
        Vec3 position{toDouble(cam["position"]["x"]), toDouble(cam["position"]["y"]), toDouble(cam["position"]["z"])};
        Vec3 rotation{toDouble(cam["rotation"]["x"]), toDouble(cam["rotation"]["y"]), toDouble(cam["rotation"]["z"])};
        double fov = toDouble(cam["fieldOfView"]);
        int width = cam["resolution"]["width"];
        int height = cam["resolution"]["height"];
        cameraOpt = Camera{position, rotation, fov, width, height};
    }

    std::optional<Supersampling> antialiasingOpt;
    if (cfg.exists("renderer")) {
        const libconfig::Setting &renderer = cfg.lookup("renderer");
        int samples = 1;
        std::string type = "uniform";
        double threshold = 0.0;

        if (renderer.exists("antialiasing")) {
            const libconfig::Setting &aa = renderer["antialiasing"];
            samples = aa["samples"];

            if (aa.exists("type"))
                type = static_cast<const char *>(aa["type"]);
            
            if (type == "adaptive" && aa.exists("threshold"))
                threshold = static_cast<double>(aa["threshold"]);
        }
        antialiasingOpt = Supersampling{samples, type, threshold};
    }

    std::unordered_map<std::string, std::string> mapTablePrim = {
        {"spheres", "sphere"},
        {"planes", "plane"},
        {"cylinders", "cylinder"},
        {"limited_cylinders", "limited_cylinder"},
        {"cones", "cone"},
        {"limited_cones", "limited_cone"},
    };

    static const std::unordered_map<std::string, std::vector<std::string>> primFields = {
        {"sphere", {"x", "y", "z", "r"}},
        {"plane", {"x", "y", "z", "nx", "ny", "nz"}},
        {"cylinder", {"x", "y", "z", "ax", "ay", "az", "r"}},
        {"limited_cylinder", {"x", "y", "z", "ax", "ay", "az", "r", "h"}},
        {"cone", {"x", "y", "z", "ax", "ay", "az", "angle"}},
        {"limited_cone", {"x", "y", "z", "ax", "ay", "az", "angle", "h"}},
    };

    auto buildMaterial = [](const libconfig::Setting &mat) -> std::shared_ptr<IMaterial> {
        std::string type = mat["type"].c_str();
        Color color{
            toDouble(mat["color"]["r"]),
            toDouble(mat["color"]["g"]),
            toDouble(mat["color"]["b"])
        };

        if (type == "flat")
            return std::make_shared<FlatColor>(color);
        if (type == "reflection")
            return std::make_shared<Reflection>(color);
        if (type == "transparency")
            return std::make_shared<Transparency>(color);
        throw std::runtime_error("Unknown material type: " + type);
    };

    Scene scene;
    PrimitiveBuilder builder(factory);

    const libconfig::Setting &primitives = cfg.lookup("primitives");
    int nbPrimitives = primitives.getLength();
    for (int i=0; i<nbPrimitives; i++) {
        const libconfig::Setting &primitive = primitives[i];
        auto it = mapTablePrim.find(primitive.getName());
        if (it == mapTablePrim.end()) {
            std::cerr << "Warning: unknown primitive group '" << primitive.getName() << "', skipping.\n";
            continue;
        }

        const std::string &factoryKey = it->second;
        const auto &fields = primFields.at(factoryKey);
        int nbValues = primitive.getLength();

        for (int j=0; j<nbValues; j++) {
            const libconfig::Setting &elem = primitive[j];

            std::unordered_map<std::string, double> params;
            try {
                for (const auto &field : fields)
                    params[field] = toDouble(elem[field.c_str()]);
            } catch (const libconfig::SettingNotFoundException &e) {
                throw std::runtime_error(std::string("Missing primitive field: ") + e.getPath());
            }

            std::shared_ptr<IMaterial> material;
            try {
                material = buildMaterial(elem["material"]);
            } catch (const libconfig::SettingNotFoundException &e) {
                throw std::runtime_error(std::string("Missing material field: ") + e.getPath());
            }
            builder.setType(factoryKey).setParams(params).setMaterial(material);

            if (elem.exists("translation")) {
                const libconfig::Setting &t = elem["translation"];
                builder.setTranslation({toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])});
            }
            if (elem.exists("rotation")) {
                const libconfig::Setting &r = elem["rotation"];
                builder.setRotation({toDouble(r["x"]), toDouble(r["y"]), toDouble(r["z"])});
            }
            if (elem.exists("scale")) {
                const libconfig::Setting &s = elem["scale"];
                builder.setScale({toDouble(s["x"]), toDouble(s["y"]), toDouble(s["z"])});
            }

            scene.addPrimitive(builder.build());
            builder.reset();
        }
    }

    const libconfig::Setting &lights = cfg.lookup("lights");
    int nbLights = lights.getLength();
    for (int i=0; i<nbLights; i++) {
        const libconfig::Setting &light = lights[i];
        std::string name = light.getName();
        int nbValues = light.getLength();

        for (int j=0; j<nbValues; j++) {
            const libconfig::Setting &elem = light[j];

            Color color{
                toDouble(elem["color"]["r"]),
                toDouble(elem["color"]["g"]),
                toDouble(elem["color"]["b"])
            };
            double intensity = toDouble(elem["intensity"]);

            if (name == "ambient") {
                scene.addLight(std::make_unique<AmbientLight>(color, intensity));
            } else if (name == "directional") {
                Vec3 direction{
                    toDouble(elem["direction"]["x"]),
                    toDouble(elem["direction"]["y"]),
                    toDouble(elem["direction"]["z"])
                };
                scene.addLight(std::make_unique<DirectionalLight>(direction, color, intensity));
            }
        }
    }

    if (cfg.exists("imports")) {
        const libconfig::Setting &imports = cfg.lookup("imports");
        for (int i=0; i<imports.getLength(); i++) {
            const libconfig::Setting &imp = imports[i];
            std::string subFilePath = imp.lookup("path").c_str();

            SceneContext importedCtx = load(subFilePath, factory, visited);

            if (!cameraOpt && importedCtx.camera) {
                cameraOpt = importedCtx.camera;
            }

            auto &importedPrims = importedCtx.scene.primitives();
            auto &importedLights = importedCtx.scene.lights();

            for (auto &prim : importedPrims) {
                PrimitivePtr p = std::move(prim);
                if (imp.exists("scale")) {
                    const libconfig::Setting &t = imp["scale"];
                    p = PrimitivePtr(new ScaleDecorator(std::move(p), Vec3{toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])}), p.get_deleter());
                }
                if (imp.exists("rotation")) {
                    const libconfig::Setting &t = imp["rotation"];
                    p = PrimitivePtr(new RotationDecorator(std::move(p), Vec3{toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])}), p.get_deleter());
                }
                if (imp.exists("translation")) {
                    const libconfig::Setting &t = imp["translation"];
                    p = PrimitivePtr(new TranslationDecorator(std::move(p), Vec3{toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])}), p.get_deleter());
                }
                scene.addPrimitive(std::move(p));
            }
            importedPrims.clear();
            
            for (auto &light : importedLights) {
                scene.addLight(std::move(light));
            }
            importedLights.clear();
        }
    }

    if (!cameraOpt) {
        throw std::runtime_error("No camera found in the configuration files");
    }
    return SceneContext{std::move(scene), cameraOpt, antialiasingOpt};
}
