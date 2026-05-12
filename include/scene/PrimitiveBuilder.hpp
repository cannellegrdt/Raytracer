/*
 * Project: Raytracer
 * File name: PrimitiveBuilder.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Builder pattern implementation for constructing primitives
 *                   with optional decorators such as translation, rotation,
 *                   scaling, shearing, and matrix transformations.
 */

#ifndef PRIMITIVEBUILDER_HPP_
    #define PRIMITIVEBUILDER_HPP_
    #include <unordered_map>
    #include <string>
    #include "Type.hpp"
    #include "Factory.hpp"
    #include "Mat4.hpp"

/// @brief Structure to hold shear factors
struct ShearFactors {
    double sxy = 0.0;  ///< Shear X by Y
    double sxz = 0.0;  ///< Shear X by Z
    double syx = 0.0;  ///< Shear Y by X
    double syz = 0.0;  ///< Shear Y by Z
    double szx = 0.0;  ///< Shear Z by X
    double szy = 0.0;  ///< Shear Z by Y
};

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

    /// @brief Sets the shear decorator.
    /// @param s Shear factors structure.
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setShear(const ShearFactors &s);

    /// @brief Sets the transform matrix decorator.
    /// @param m Matrix.
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setTransformMatrix(const Mat4 &m);

    /// @brief Sets the geometry parameters passed to configure().
    /// @param params Map of parameter names to values (e.g. {"x",0}, {"r",1}).
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setParams(const std::unordered_map<std::string, double> &params);

    /// @brief Sets the source file path for file-based primitives (e.g. OBJ meshes).
    /// @param path Path to the source file.
    /// @return Reference to this builder for chaining.
    PrimitiveBuilder &setFile(const std::string &path);

    /// @brief Builds the primitive with all configured options.
    /// @return Unique pointer to the constructed primitive.
    PrimitivePtr build();

    /// @brief Resets the builder to initial state.
    void reset();

private:
    PrimitiveFactory &_factory;                      ///< Factory for creating primitives.
    std::string _type;                               ///< Primitive type identifier.
    std::shared_ptr<IMaterial> _material;            ///< Material for the primitive.
    std::optional<Vec3> _translation;                ///< Translation vector.
    std::optional<Vec3> _rotation;                   ///< Rotation angles.
    std::optional<Vec3> _scale;                      ///< Scale factors.
    std::optional<ShearFactors> _shear;              ///< Shear factors.
    std::optional<Mat4> _transformMatrix;            ///< 4x4 matrix.
    std::unordered_map<std::string, double> _params; ///< Geometry parameters for configure().
    std::optional<std::string> _filePath;            ///< Source file path for file-based primitives.
};

#endif /* PRIMITIVEBUILDER_HPP_ */
