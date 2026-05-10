/*
 * Project: Raytracer
 * File name: MaterialBuilder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of MaterialBuilder for creating materials from libconfig settings.
 */

#include <libconfig.h++>
#include <memory>
#include "MaterialBuilder.hpp"
#include "ConfigUtils.hpp"
#include "FlatColor.hpp"
#include "Transparency.hpp"
#include "Reflection.hpp"
#include "Refraction.hpp"
#include "PhongMaterial.hpp"
#include "TexturedMaterial.hpp"
#include "ProceduralCheckerboard.hpp"
#include "ProceduralMarble.hpp"
#include "NormalMapMaterial.hpp"

std::shared_ptr<IMaterial> MaterialBuilder::build(const libconfig::Setting &mat) {
    std::string type = mat["type"].c_str();
    if (type == "textured") {
        std::string texturePath = mat["texture"].c_str();
        return std::make_shared<TexturedMaterial>(texturePath);
    }
    if (type == "chessboard" || type == "marble") {
        Color colorA;
        Color colorB;
        double scale = 1.0;
        double turbulence = 1.0;
        int octaves = 1.0;
        if (mat.exists("colorA")) {
            colorA = {
                ConfigUtils::toDouble(mat["colorA"]["r"]),
                ConfigUtils::toDouble(mat["colorA"]["g"]),
                ConfigUtils::toDouble(mat["colorA"]["b"])
            };
        }
        if (mat.exists("colorB")) {
            colorB = {
                ConfigUtils::toDouble(mat["colorB"]["r"]),
                ConfigUtils::toDouble(mat["colorB"]["g"]),
                ConfigUtils::toDouble(mat["colorB"]["b"])
            };
        }
        if (mat.exists("scale"))
            scale = ConfigUtils::toDouble(mat["scale"]);
        if (mat.exists("turbulence"))
            turbulence = ConfigUtils::toDouble(mat["turbulence"]);
        if (mat.exists("octaves"))
            octaves = ConfigUtils::toDouble(mat["octaves"]);
        
        if (type == "chessboard")
            return std::make_shared<ProceduralCheckerboard>(colorA, colorB, scale);
        return std::make_shared<ProceduralMarble>(colorA, colorB, scale, turbulence, octaves);
    }
    if (type == "normalmap") {
        std::string normalMapPath = mat["normalmap"].c_str();
        
        if (!mat.exists("base"))
            throw std::runtime_error("NormalMap material requires a 'base' material definition");
        auto baseMaterial = build(mat["base"]);
        
        return std::make_shared<NormalMapMaterial>(baseMaterial, normalMapPath);
    }
    Color color{
        ConfigUtils::toDouble(mat["color"]["r"]),
        ConfigUtils::toDouble(mat["color"]["g"]),
        ConfigUtils::toDouble(mat["color"]["b"])
    };
    if (type == "flat")
        return std::make_shared<FlatColor>(color);
    if (type == "reflection")
        return std::make_shared<Reflection>(color);
    if (type == "transparency")
        return std::make_shared<Transparency>(color);
    if (type == "phong") {
        Color specColor{
            ConfigUtils::toDouble(mat["specular"]["r"]),
            ConfigUtils::toDouble(mat["specular"]["g"]),
            ConfigUtils::toDouble(mat["specular"]["b"])
        };
        return std::make_shared<PhongMaterial>(color, specColor, ConfigUtils::toDouble(mat["shininess"]));
    }
    if (type == "refraction") {
        double ior = 1.5;
        if (mat.exists("ior"))
            ior = ConfigUtils::toDouble(mat["ior"]);
        if (ior <= 0.0)
            throw std::runtime_error("IOR must be > 0");
        return std::make_shared<Refraction>(color, ior);
    }
    throw std::runtime_error("Unknown material type: " + type);
}