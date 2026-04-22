/*
 * Project: Raytracer
 * File name: PrimitiveBuilder.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Builder pattern for constructing primitives with optional decorators.
 */

#ifndef PRIMITIVEBUILDER_HPP_
    #define PRIMITIVEBUILDER_HPP_
    #include <unordered_map>
    #include <string>
    #include "Type.hpp"
    #include "Factory.hpp"

/// @brief Builder for constructing primitives with decorators.
/// @details Provides a fluent API to set primitive type, material, and optional
/// transforms (translation, rotation, scale) before building.
class PrimitiveBuilder {
public:
    /// @brief Constructs a builder attached to a factory.
    /// @param factory Factory for creating base primitives.
    explicit PrimitiveBuilder(PrimitiveFactory &factory) : _factory(factory) {}

    /// @brief Sets the primitive type.
    /// @param type Primitive type identifier (e.g., "sphere", "plane").
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setType(const std::string &type);

    /// @brief Sets the material for the primitive.
    /// @param material Shared pointer to material.
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setMaterial(std::shared_ptr<IMaterial> material);

    /// @brief Sets the translation decorator.
    /// @param t Translation vector.
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setTranslation(const Vec3 &t);

    /// @brief Sets the rotation decorator.
    /// @param r Rotation angles in radians (X, Y, Z).
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setRotation(const Vec3 &r);

    /// @brief Sets the scale decorator.
    /// @param s Scale factors (X, Y, Z).
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setScale(const Vec3 &s);

    /// @brief Sets the geometry parameters passed to configure().
    /// @param params Map of parameter names to values (e.g. {"x",0}, {"r",1}).
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setParams(const std::unordered_map<std::string, double> &params);

    /// @brief Builds the primitive with all configured options.
    /// @return Unique pointer to the constructed primitive.
    PrimitivePtr build();

    /// @brief Resets the builder to initial state.
    void reset();

private:
    PrimitiveFactory &_factory;                        ///< Factory for creating primitives.
    std::string _type;                                 ///< Primitive type identifier.
    std::shared_ptr<IMaterial> _material;              ///< Material for the primitive.
    std::optional<Vec3> _translation;                  ///< Translation vector.
    std::optional<Vec3> _rotation;                     ///< Rotation angles.
    std::optional<Vec3> _scale;                        ///< Scale factors.
    std::unordered_map<std::string, double> _params;   ///< Geometry parameters for configure().
};

#endif /* PRIMITIVEBUILDER_HPP_ */
