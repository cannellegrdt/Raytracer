/*
 * Project: Raytracer
 * File name: MaterialBuilder.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Builder for creating materials from libconfig settings.
 */

#ifndef MATERIALBUILDER_HPP_
    #define MATERIALBUILDER_HPP_

#include <libconfig.h++>
#include <memory>
#include "IMaterial.hpp"

/// @brief Builder class for creating materials from libconfig settings.
class MaterialBuilder {
public:
    /// @brief Builds a material from a libconfig setting.
    /// @param mat The libconfig setting containing material definition.
    /// @return Shared pointer to the created material.
    static std::shared_ptr<IMaterial> build(const libconfig::Setting &mat);
};

#endif /* MATERIALBUILDER_HPP_ */