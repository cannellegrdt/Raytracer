/*
 * Project: Raytracer
 * File name: ConfigUtils.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of ConfigUtils helper functions.
 */

#include <libconfig.h++>
#include <optional>
#include <functional>
#include <stdexcept>
#include "ConfigUtils.hpp"

double ConfigUtils::toDouble(const libconfig::Setting &s) {
    if (s.getType() == libconfig::Setting::TypeInt)
        return static_cast<double>((int)s);
    if (s.getType() == libconfig::Setting::TypeInt64)
        return static_cast<double>((long long)s);
    if (s.getType() == libconfig::Setting::TypeFloat)
        return static_cast<double>((double)s);
    throw std::runtime_error(std::string("Expected numeric setting at: ") + s.getPath());
}

std::optional<std::reference_wrapper<const libconfig::Setting>> ConfigUtils::checkValues(
    const libconfig::Config& cfg, const std::string& path) {
    if (cfg.exists(path))
        return std::ref(cfg.lookup(path));
    return std::nullopt;
}