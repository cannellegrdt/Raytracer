/*
 * Project: Raytracer
 * File name: LightsParser.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Parser for lights from libconfig settings.
 */

#ifndef LIGHTSPARSER_HPP_
    #define LIGHTSPARSER_HPP_

#include <libconfig.h++>
#include <memory>
#include "Scene.hpp"

/// @brief Parser class for creating lights from libconfig settings.
class LightsParser {
public:
    /// @brief Parses lights from a libconfig setting and adds them to the scene.
    /// @param lights The libconfig setting containing lights definitions.
    /// @param scene The scene to add lights to.
    static void parseAndAddLights(const libconfig::Setting &lights, Scene &scene);
};

#endif /* LIGHTSPARSER_HPP_ */