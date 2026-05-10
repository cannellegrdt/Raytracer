/*
 * Project: Raytracer
 * File name: RendererParser.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Parser for renderer settings from libconfig settings.
 */

#ifndef RENDERERPARSER_HPP_
    #define RENDERERPARSER_HPP_

#include <libconfig.h++>
#include <optional>
#include "Renderer.hpp"

/// @brief Parser class for renderer settings from libconfig settings.
class RendererParser {
public:
    /// @brief Parses antialiasing settings from a libconfig configuration.
    /// @param cfg The libconfig configuration.
    /// @return Optional supersampling settings if found.
    static std::optional<Supersampling> parseAntialiasing(const libconfig::Config &cfg);

    /// @brief Parses AO rays setting from a libconfig configuration.
    /// @param cfg The libconfig configuration.
    /// @return Optional number of AO rays if found.
    static std::optional<int> parseAORays(const libconfig::Config &cfg);
};

#endif /* RENDERERPARSER_HPP_ */