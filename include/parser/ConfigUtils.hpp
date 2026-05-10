/*
 * Project: Raytracer
 * File name: ConfigUtils.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Utility class for libconfig++ helper functions.
 */

#ifndef CONFIGUTILS_HPP_
    #define CONFIGUTILS_HPP_

#include <libconfig.h++>
#include <optional>
#include <functional>

/// @brief Utility class for libconfig++ helper functions.
class ConfigUtils {
public:
    /// @brief Converts a libconfig setting to double.
    /// @param s The libconfig setting to convert.
    /// @return The double value.
    static double toDouble(const libconfig::Setting &s);

    /// @brief Checks if a path exists in config and returns a reference to it.
    /// @param cfg The libconfig configuration.
    /// @param path The path to check.
    /// @return Optional reference wrapper to the setting if it exists.
    static std::optional<std::reference_wrapper<const libconfig::Setting>> checkValues(const libconfig::Config& cfg, const std::string& path);
};

#endif /* CONFIGUTILS_HPP_ */