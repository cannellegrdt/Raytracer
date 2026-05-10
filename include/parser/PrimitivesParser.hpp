/*
 * Project: Raytracer
 * File name: PrimitivesParser.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Parser for primitives from libconfig settings.
 */

#ifndef PRIMITIVESPARSER_HPP_
    #define PRIMITIVESPARSER_HPP_

#include <libconfig.h++>
#include <vector>
#include <memory>
#include "Type.hpp"
#include "PrimitiveBuilder.hpp"

/// @brief Parser class for creating primitives from libconfig settings.
class PrimitivesParser {
public:
    /// @brief Parses a primitives block from libconfig.
    /// @param primBlock The libconfig setting containing primitives definitions.
    /// @param builder The primitive builder to use for creating primitives.
    /// @return Vector of parsed primitives.
    static std::vector<PrimitivePtr> parse(const libconfig::Setting &primBlock, PrimitiveBuilder &builder);
};

#endif /* PRIMITIVESPARSER_HPP_ */