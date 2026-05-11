/*
 * Project: Raytracer
 * File name: PrimitivesParser.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of PrimitivesParser for parsing primitives from libconfig settings.
 */

#include <libconfig.h++>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "PrimitivesParser.hpp"
#include "PrimitiveBuilder.hpp"
#include "ConfigUtils.hpp"
#include "MaterialBuilder.hpp"
#include "TransformBuilder.hpp"
#include "GroupNode.hpp"

static const std::unordered_map<std::string, std::string> kMapTablePrim = {
    {"spheres", "sphere"},
    {"planes", "plane"},
    {"cylinders", "cylinder"},
    {"limited_cylinders", "limited_cylinder"},
    {"cones", "cone"},
    {"limited_cones", "limited_cone"},
    {"torus", "torus"},
    {"cubes", "cube"},
    {"tanglecubes", "tanglecube"},
    {"triangles", "triangle"},
    {"mandelbulbs", "mandelbulb"},
    {"mobius", "mobius"},
};

static const std::unordered_map<std::string, std::vector<std::string>> kPrimFields = {
    {"sphere", {"x", "y", "z", "r"}},
    {"plane", {"x", "y", "z", "nx", "ny", "nz"}},
    {"cylinder", {"x", "y", "z", "ax", "ay", "az", "r"}},
    {"limited_cylinder", {"x", "y", "z", "ax", "ay", "az", "r", "h"}},
    {"cone", {"x", "y", "z", "ax", "ay", "az", "angle"}},
    {"limited_cone", {"x", "y", "z", "ax", "ay", "az", "angle", "h"}},
    {"torus", {"x", "y", "z", "ax", "ay", "az", "R", "r"}},
    {"cube", {"x", "y", "z", "s"}},
    {"tanglecube", {"x", "y", "z", "s"}},
    {"triangle", {"v0x", "v0y", "v0z", "v1x", "v1y", "v1z", "v2x", "v2y", "v2z"}},
    {"mandelbulb", {"x", "y", "z", "s", "power", "iters", "bailout"}},
    {"mobius", {"x", "y", "z", "R", "w"}},
};

static void applyTransformations(PrimitiveBuilder &builder, const libconfig::Setting &elem) {
    if (elem.exists("transformation_matrix")) {
        const libconfig::Setting &tm = elem["transformation_matrix"];
        Mat4 matrix{};
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                matrix.m[row][col] = ConfigUtils::toDouble(tm[row][col]);
        builder.setTransformMatrix(matrix);
    }
    if (elem.exists("translation")) {
        const libconfig::Setting &t = elem["translation"];
        builder.setTranslation({
            ConfigUtils::toDouble(t["x"]),
            ConfigUtils::toDouble(t["y"]),
            ConfigUtils::toDouble(t["z"])
        });
    }
    if (elem.exists("rotation")) {
        const libconfig::Setting &r = elem["rotation"];
        builder.setRotation({
            ConfigUtils::toDouble(r["x"]),
            ConfigUtils::toDouble(r["y"]),
            ConfigUtils::toDouble(r["z"])
        });
    }
    if (elem.exists("shear")) {
        const libconfig::Setting &sh = elem["shear"];
        builder.setShear({
            ConfigUtils::toDouble(sh["sxy"]), ConfigUtils::toDouble(sh["sxz"]),
            ConfigUtils::toDouble(sh["syx"]), ConfigUtils::toDouble(sh["syz"]),
            ConfigUtils::toDouble(sh["szx"]), ConfigUtils::toDouble(sh["szy"])
        });
    }
    if (elem.exists("scale")) {
        const libconfig::Setting &s = elem["scale"];
        builder.setScale({
            ConfigUtils::toDouble(s["x"]),
            ConfigUtils::toDouble(s["y"]),
            ConfigUtils::toDouble(s["z"])
        });
    }
}

std::vector<PrimitivePtr> PrimitivesParser::parse(const libconfig::Setting &primBlock, PrimitiveBuilder &builder) {
    std::vector<PrimitivePtr> result;

    for (int i = 0; i < primBlock.getLength(); i++) {
        const libconfig::Setting &section = primBlock[i];
        std::string sectionName = section.getName();

        if (sectionName == "groups") {
            for (int j = 0; j < section.getLength(); j++) {
                const libconfig::Setting &grp = section[j];

                std::vector<PrimitivePtr> children;
                if (grp.exists("children"))
                    children = parse(grp["children"], builder);

                PrimitivePtr groupPtr = makeGroup(std::move(children));
                result.push_back(TransformBuilder::applyTransforms(std::move(groupPtr), grp));
            }
            continue;
        }

        if (sectionName == "obj_meshes") {
            for (int j = 0; j < section.getLength(); j++) {
                const libconfig::Setting &elem = section[j];

                if (!elem.exists("file"))
                    throw std::runtime_error("OBJ mesh entry missing required 'file' field");
                std::string filePath = elem["file"].c_str();

                std::shared_ptr<IMaterial> material;
                try {
                    material = MaterialBuilder::build(elem["material"]);
                } catch (const libconfig::SettingNotFoundException &e) {
                    throw std::runtime_error(std::string("Missing material field: ") + e.getPath());
                }

                builder.setType("obj").setFile(filePath).setMaterial(material);

                applyTransformations(builder, elem);

                result.push_back(builder.build());
                builder.reset();
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
                    params[field] = ConfigUtils::toDouble(elem[field.c_str()]);
            } catch (const libconfig::SettingNotFoundException &e) {
                throw std::runtime_error(std::string("Missing primitive field: ") + e.getPath());
            }

            std::shared_ptr<IMaterial> material;
            try {
                material = MaterialBuilder::build(elem["material"]);
            } catch (const libconfig::SettingNotFoundException &e) {
                throw std::runtime_error(std::string("Missing material field: ") + e.getPath());
            }

            builder.setType(factoryKey).setParams(params).setMaterial(material);

            applyTransformations(builder, elem);

            result.push_back(builder.build());
            builder.reset();
        }
    }

    return result;
}