/*
 * Project: Raytracer
 * File name: LibconfigLoader.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Scene loading implementation; reads camera, primitives and lights from a libconfig++ .cfg file into a SceneContext.
 */

#include <string>
#include <libconfig.h++>
#include <optional>
#include <functional>
#include <vector>
#include <unordered_map>
#include "FlatColor.hpp"
#include "Transparency.hpp"
#include "Reflection.hpp"
#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "LibconfigLoader.hpp"
#include "PrimitiveBuilder.hpp"

static double toDouble(const libconfig::Setting &s) {
    if (s.getType() == libconfig::Setting::TypeInt)
        return static_cast<double>((int)s);
    if (s.getType() == libconfig::Setting::TypeInt64)
        return static_cast<double>((long long)s);
    return (double)s;
}

static std::optional<std::reference_wrapper<const libconfig::Setting>> checkValues(const libconfig::Config& cfg, const std::string& path) {
    if (cfg.exists(path))
        return std::ref(cfg.lookup(path));
    return std::nullopt;
}

SceneContext LibconfigLoader::load(const std::string &filePath, PrimitiveFactory &factory) {
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
        "camera",
        "primitives",
        "lights",
        "camera.resolution.width", "camera.resolution.height",
        "camera.position.x", "camera.position.y", "camera.position.z",
        "camera.rotation.x", "camera.rotation.y", "camera.rotation.z",
        "camera.fieldOfView"
    };
    
    for (const auto &key : requiredKeys) {
        auto setting = checkValues(cfg, key);
        if (!setting)
            throw std::runtime_error("Missing key: " + key);
    }

    const libconfig::Setting &cam = cfg.lookup("camera");
    Vec3 position{cam["position"]["x"], cam["position"]["y"], cam["position"]["z"]};
    Vec3 rotation{cam["rotation"]["x"], cam["rotation"]["y"], cam["rotation"]["z"]};
    double fov = cam["fieldOfView"];
    int width = cam["resolution"]["width"];
    int height = cam["resolution"]["height"];
    Camera camera{position, rotation, fov, width, height};

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
        if (it == mapTablePrim.end()) continue;

        const std::string &factoryKey = it->second;
        const auto &fields = primFields.at(factoryKey);
        int nbValues = primitive.getLength();

        for (int j=0; j<nbValues; j++) {
            const libconfig::Setting &elem = primitive[j];

            std::unordered_map<std::string, double> params;
            for (const auto &field : fields)
                params[field] = toDouble(elem[field.c_str()]);
            
            auto material = buildMaterial(elem["material"]);
            scene.addPrimitive(
                builder.setType(factoryKey).setParams(params).setMaterial(material).build()
            );
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
    return SceneContext{std::move(scene), camera};
}
