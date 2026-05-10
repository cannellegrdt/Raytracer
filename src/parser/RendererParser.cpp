/*
 * Project: Raytracer
 * File name: RendererParser.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of RendererParser for parsing renderer settings from libconfig settings.
 */

#include <libconfig.h++>
#include <optional>
#include "RendererParser.hpp"
#include "Renderer.hpp"

std::optional<Supersampling> RendererParser::parseAntialiasing(const libconfig::Config &cfg) {
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
        return Supersampling{samples, type, threshold};
    }
    return std::nullopt;
}

std::optional<int> RendererParser::parseAORays(const libconfig::Config &cfg) {
    if (cfg.exists("renderer")) {
        const libconfig::Setting &renderer = cfg.lookup("renderer");
        if (renderer.exists("nbAORays"))
            return static_cast<int>(renderer["nbAORays"]);
    }
    return std::nullopt;
}