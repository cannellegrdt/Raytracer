/*
 * Project: Raytracer
 * File name: CameraParser.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of CameraParser for parsing camera from libconfig settings.
 */

#include <libconfig.h++>
#include <optional>
#include "CameraParser.hpp"
#include "ConfigUtils.hpp"
#include "Camera.hpp"

std::optional<Camera> CameraParser::parse(const libconfig::Config &cfg) {
    if (cfg.exists("camera")) {
        const libconfig::Setting &cam = cfg.lookup("camera");
        Vec3 position{
            ConfigUtils::toDouble(cam["position"]["x"]),
            ConfigUtils::toDouble(cam["position"]["y"]),
            ConfigUtils::toDouble(cam["position"]["z"])
        };
        Vec3 rotation{
            ConfigUtils::toDouble(cam["rotation"]["x"]),
            ConfigUtils::toDouble(cam["rotation"]["y"]),
            ConfigUtils::toDouble(cam["rotation"]["z"])
        };
        double fov = ConfigUtils::toDouble(cam["fieldOfView"]);
        int width = cam["resolution"]["width"];
        int height = cam["resolution"]["height"];
        
        return Camera{position, rotation, fov, width, height};
    }
    return std::nullopt;
}