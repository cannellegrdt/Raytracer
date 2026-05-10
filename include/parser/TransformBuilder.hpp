/*
 * Project: Raytracer
 * File name: TransformBuilder.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Builder for applying transformations to primitives from libconfig settings.
 */

#ifndef TRANSFORMBUILDER_HPP_
    #define TRANSFORMBUILDER_HPP_

#include <libconfig.h++>
#include <memory>
#include "Type.hpp"

/// @brief Builder class for applying transformations to primitives from libconfig settings.
class TransformBuilder {
public:
    /// @brief Applies transformations to a primitive based on libconfig settings.
    /// @param p The primitive to transform.
    /// @param elem The libconfig setting containing transformation definitions.
    /// @return The transformed primitive.
    static PrimitivePtr applyTransforms(PrimitivePtr p, const libconfig::Setting &elem);
};

#endif /* TRANSFORMBUILDER_HPP_ */