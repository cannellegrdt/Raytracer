/*
 * Project: Raytracer
 * File name: LightsParser.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of LightsParser for parsing lights from libconfig settings.
 */

#include <libconfig.h++>
#include <memory>
#include "LightsParser.hpp"
#include "ConfigUtils.hpp"
#include "Scene.hpp"
#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"

void LightsParser::parseAndAddLights(const libconfig::Setting &lights, Scene &scene) {
    int nbLights = lights.getLength();
    for (int i = 0; i < nbLights; i++) {
        const libconfig::Setting &light = lights[i];
        std::string name = light.getName();
        int nbValues = light.getLength();

        for (int j = 0; j < nbValues; j++) {
            const libconfig::Setting &elem = light[j];

            Color color{
                ConfigUtils::toDouble(elem["color"]["r"]),
                ConfigUtils::toDouble(elem["color"]["g"]),
                ConfigUtils::toDouble(elem["color"]["b"])
            };
            double intensity = ConfigUtils::toDouble(elem["intensity"]);

            if (name == "ambient") {
                double maxDist = 10.0;
                if (elem.exists("maxDist"))
                    maxDist = ConfigUtils::toDouble(elem["maxDist"]);
                scene.addLight(std::make_unique<AmbientLight>(color, intensity, maxDist));
            } else if (name == "directional") {
                Vec3 direction{
                    ConfigUtils::toDouble(elem["direction"]["x"]),
                    ConfigUtils::toDouble(elem["direction"]["y"]),
                    ConfigUtils::toDouble(elem["direction"]["z"])
                };
                scene.addLight(std::make_unique<DirectionalLight>(direction, color, intensity));
            } else if (name == "point") {
                Vec3 position{
                    ConfigUtils::toDouble(elem["position"]["x"]),
                    ConfigUtils::toDouble(elem["position"]["y"]),
                    ConfigUtils::toDouble(elem["position"]["z"])
                };
                scene.addLight(std::make_unique<PointLight>(position, color, intensity));
            }
        }
    }
}