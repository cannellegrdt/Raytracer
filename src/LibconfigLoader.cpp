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
#include "Refraction.hpp"
#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "PhongMaterial.hpp"
#include "LibconfigLoader.hpp"
#include "PrimitiveBuilder.hpp"
#include "Decorators.hpp"
#include "GroupNode.hpp"
#include "Renderer.hpp"
#include "Mat4.hpp"

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

static std::shared_ptr<IMaterial> buildMaterial(const libconfig::Setting &mat) {
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
    if (type == "phong") {
        Color specColor{
            toDouble(mat["specular"]["r"]),
            toDouble(mat["specular"]["g"]),
            toDouble(mat["specular"]["b"])
        };
        return std::make_shared<PhongMaterial>(color, specColor, mat["shininess"]);
    }
    if (type == "refraction") {
        double ior = 1.5;
        if (mat.exists("ior"))
            ior = toDouble(mat["ior"]);
        if (ior <= 0.0)
            throw std::runtime_error("IOR must be > 0");
        return std::make_shared<Refraction>(color, ior);
    }
    throw std::runtime_error("Unknown material type: " + type);
}

static PrimitivePtr applyTransforms(PrimitivePtr p, const libconfig::Setting &elem) {
    if (elem.exists("scale")) {
        const libconfig::Setting &s = elem["scale"];
        p = PrimitivePtr(
            new ScaleDecorator(std::move(p), Vec3{toDouble(s["x"]), toDouble(s["y"]), toDouble(s["z"])}),
            &defaultDestroy<ScaleDecorator>
        );
    }
    if (elem.exists("shear")) {
        const libconfig::Setting &sh = elem["shear"];
        p = PrimitivePtr(
            new ShearDecorator(std::move(p),
                toDouble(sh["sxy"]), toDouble(sh["sxz"]),
                toDouble(sh["syx"]), toDouble(sh["syz"]),
                toDouble(sh["szx"]), toDouble(sh["szy"])),
            &defaultDestroy<ShearDecorator>
        );
    }
    if (elem.exists("rotation")) {
        const libconfig::Setting &r = elem["rotation"];
        p = PrimitivePtr(
            new RotationDecorator(std::move(p), Vec3{toDouble(r["x"]), toDouble(r["y"]), toDouble(r["z"])}),
            &defaultDestroy<RotationDecorator>
        );
    }
    if (elem.exists("translation")) {
        const libconfig::Setting &t = elem["translation"];
        p = PrimitivePtr(
            new TranslationDecorator(std::move(p), Vec3{toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])}),
            &defaultDestroy<TranslationDecorator>
        );
    }
    if (elem.exists("transformation_matrix")) {
        const libconfig::Setting &tm = elem["transformation_matrix"];
        Mat4 matrix{};
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                matrix.m[row][col] = toDouble(tm[row][col]);
        p = PrimitivePtr(
            new TransformMatrixDecorator(std::move(p), matrix),
            &defaultDestroy<TransformMatrixDecorator>
        );
    }
    return p;
}

static const std::unordered_map<std::string, std::string> kMapTablePrim = {
    {"spheres", "sphere"},
    {"planes", "plane"},
    {"cylinders", "cylinder"},
    {"limited_cylinders", "limited_cylinder"},
    {"cones", "cone"},
    {"limited_cones", "limited_cone"},
};

static const std::unordered_map<std::string, std::vector<std::string>> kPrimFields = {
    {"sphere", {"x", "y", "z", "r"}},
    {"plane", {"x", "y", "z", "nx", "ny", "nz"}},
    {"cylinder", {"x", "y", "z", "ax", "ay", "az", "r"}},
    {"limited_cylinder", {"x", "y", "z", "ax", "ay", "az", "r", "h"}},
    {"cone", {"x", "y", "z", "ax", "ay", "az", "angle"}},
    {"limited_cone", {"x", "y", "z", "ax", "ay", "az", "angle", "h"}},
};

static std::vector<PrimitivePtr> parsePrimitivesBlock(
    const libconfig::Setting &primBlock,
    PrimitiveBuilder &builder
) {
    std::vector<PrimitivePtr> result;

    for (int i = 0; i < primBlock.getLength(); i++) {
        const libconfig::Setting &section = primBlock[i];
        std::string sectionName = section.getName();

        if (sectionName == "groups") {
            for (int j = 0; j < section.getLength(); j++) {
                const libconfig::Setting &grp = section[j];

                std::vector<PrimitivePtr> children;
                if (grp.exists("children"))
                    children = parsePrimitivesBlock(grp["children"], builder);

                PrimitivePtr groupPtr = makeGroup(std::move(children));
                result.push_back(applyTransforms(std::move(groupPtr), grp));
            }
            continue;
        }

        auto it = kMapTablePrim.find(sectionName);
        if (it == kMapTablePrim.end()) {
            std::cerr << "Warning: unknown primitive group '" << sectionName << "', skipping.\n";
            continue;
        }

        const std::string &factoryKey = it->second;
        const auto &fields = kPrimFields.at(factoryKey);

        for (int j = 0; j < section.getLength(); j++) {
            const libconfig::Setting &elem = section[j];

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

            if (elem.exists("transformation_matrix")) {
                const libconfig::Setting &tm = elem["transformation_matrix"];
                Mat4 matrix{};
                for (int row = 0; row < 4; row++)
                    for (int col = 0; col < 4; col++)
                        matrix.m[row][col] = toDouble(tm[row][col]);
                builder.setTransformMatrix(matrix);
            }
            if (elem.exists("translation")) {
                const libconfig::Setting &t = elem["translation"];
                builder.setTranslation({toDouble(t["x"]), toDouble(t["y"]), toDouble(t["z"])});
            }
            if (elem.exists("rotation")) {
                const libconfig::Setting &r = elem["rotation"];
                builder.setRotation({toDouble(r["x"]), toDouble(r["y"]), toDouble(r["z"])});
            }
            if (elem.exists("shear")) {
                const libconfig::Setting &sh = elem["shear"];
                builder.setShear({toDouble(sh["sxy"]), toDouble(sh["sxz"]),
                    toDouble(sh["syx"]), toDouble(sh["syz"]),
                    toDouble(sh["szx"]), toDouble(sh["szy"])});
            }
            if (elem.exists("scale")) {
                const libconfig::Setting &s = elem["scale"];
                builder.setScale({toDouble(s["x"]), toDouble(s["y"]), toDouble(s["z"])});
            }

            result.push_back(builder.build());
            builder.reset();
        }
    }

    return result;
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

    for (const auto &key : {"primitives", "lights"}) {
        if (!checkValues(cfg, key))
            throw std::runtime_error(std::string("Missing key: ") + key);
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

    Scene scene;
    PrimitiveBuilder builder(factory);

    auto prims = parsePrimitivesBlock(cfg.lookup("primitives"), builder);
    for (auto &p : prims)
        scene.addPrimitive(std::move(p));

    const libconfig::Setting &lights = cfg.lookup("lights");
    int nbLights = lights.getLength();
    for (int i = 0; i < nbLights; i++) {
        const libconfig::Setting &light = lights[i];
        std::string name = light.getName();
        int nbValues = light.getLength();

        for (int j = 0; j < nbValues; j++) {
            const libconfig::Setting &elem = light[j];

            Color color{
                toDouble(elem["color"]["r"]),
                toDouble(elem["color"]["g"]),
                toDouble(elem["color"]["b"])
            };
            double intensity = toDouble(elem["intensity"]);

            if (name == "ambient") {
                double maxDist = 10.0;
                if (elem.exists("maxDist"))
                    maxDist = toDouble(elem["maxDist"]);
                scene.addLight(std::make_unique<AmbientLight>(color, intensity, maxDist));
            } else if (name == "directional") {
                Vec3 direction{
                    toDouble(elem["direction"]["x"]),
                    toDouble(elem["direction"]["y"]),
                    toDouble(elem["direction"]["z"])
                };
                scene.addLight(std::make_unique<DirectionalLight>(direction, color, intensity));
            } else if (name == "point") {
                Vec3 position{
                    toDouble(elem["position"]["x"]),
                    toDouble(elem["position"]["y"]),
                    toDouble(elem["position"]["z"])
                };
                scene.addLight(std::make_unique<PointLight>(position, color, intensity));
            }
        }
    }

    if (cfg.exists("imports")) {
        const libconfig::Setting &imports = cfg.lookup("imports");
        for (int i = 0; i < imports.getLength(); i++) {
            const libconfig::Setting &imp = imports[i];
            std::string subFilePath = imp.lookup("path").c_str();

            SceneContext importedCtx = load(subFilePath, factory, visited);

            if (!cameraOpt && importedCtx.camera)
                cameraOpt = importedCtx.camera;

            auto &importedPrims = importedCtx.scene.primitives();
            for (auto &prim : importedPrims)
                scene.addPrimitive(applyTransforms(std::move(prim), imp));
            importedPrims.clear();

            auto &importedLights = importedCtx.scene.lights();
            for (auto &light : importedLights)
                scene.addLight(std::move(light));
            importedLights.clear();
        }
    }

    if (cfg.exists("background")) {
        const libconfig::Setting &bg = cfg.lookup("background");
        scene.setBackgroundColor(Color{
            toDouble(bg["r"]),
            toDouble(bg["g"]),
            toDouble(bg["b"])
        });
    }

    if (!cameraOpt)
        throw std::runtime_error("No camera found in the configuration files");
    return SceneContext{std::move(scene), cameraOpt, antialiasingOpt};
}
