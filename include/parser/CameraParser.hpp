/*
 * Project: Raytracer
 * File name: CameraParser.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Parser for camera from libconfig settings.
 */

#ifndef CAMERAPARSER_HPP_
    #define CAMERAPARSER_HPP_

#include <libconfig.h++>
#include <optional>
#include "Camera.hpp"

/// @brief Parser class for creating camera from libconfig settings.
class CameraParser {
public:
    /// @brief Parses camera from a libconfig setting.
    /// @param cfg The libconfig configuration.
    /// @return Optional camera if found in config.
    static std::optional<Camera> parse(const libconfig::Config &cfg);
};

#endif /* CAMERAPARSER_HPP_ */