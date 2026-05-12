/*
 * Project: Raytracer
 * File name: Decorators.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Decorator pattern implementation for transforming primitives
 *                  with translation, rotation, scaling, shearing, and matrix transformations.
 */

#ifndef DECORATORS_HPP_
    #define DECORATORS_HPP_
    #include "Type.hpp"
    #include "Mat3.hpp"
    #include "Mat4.hpp"

/// @brief Decorator that translates a primitive by a fixed offset.
class TranslationDecorator : public IPrimitive {
public:
    /// @brief Constructs a translation decorator.
    /// @param inner Primitive to wrap.
    /// @param offset Translation vector.
    TranslationDecorator(PrimitivePtr inner, const Vec3 &offset);
    void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner; ///< Wrapped primitive being decorated.
    Vec3 _offset;        ///< Translation offset vector (x, y, z).
};

/// @brief Decorator that rotates a primitive around a center point.
class RotationDecorator : public IPrimitive {
public:
    /// @brief Constructs a rotation decorator.
    /// @param inner Primitive to wrap.
    /// @param angles Rotation angles (radians) for X, Y, Z.
    RotationDecorator(PrimitivePtr inner, const Vec3 &angles);
    void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner; ///< Wrapped primitive being decorated.
    Vec3 _angles;        ///< Rotation angles in radians (X, Y, Z).
    Mat3 _rotation;      ///< Rotation matrix for transforming points.
    Mat3 _invRotation;   ///< Inverse rotation matrix for transforming normals.
};

/// @brief Decorator that scales a primitive.
class ScaleDecorator : public IPrimitive {
public:
    /// @brief Constructs a scale decorator.
    /// @param inner Primitive to wrap.
    /// @param scale Scale factors for X, Y, Z.
    ScaleDecorator(PrimitivePtr inner, const Vec3 &scale);
    void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner; ///< Wrapped primitive being decorated.
    Vec3 _scale;         ///< Scale factors for X, Y, Z axes.
};

/// @brief Decorator that shear a primitive around a center point.
class ShearDecorator : public IPrimitive {
public:
    /// @brief Constructs a shear decorator.
    /// @param inner Primitive to wrap.
    /// @param sxy, sxz, syx, syz, szx, szy Shear factors
    ShearDecorator(PrimitivePtr inner, double sxy, double sxz, double syx, double syz, double szx, double szy);
    void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;      ///< Wrapped primitive being decorated.
    Mat3 _shear;              ///< Shear matrix for transforming points.
    Mat3 _invShear;           ///< Inverse shear matrix.
    Mat3 _invShearTransposed; ///< Transposed inverse shear matrix for normal transformation.
};

/// @brief Decorator that transform a primitive with a 4x4 matrix.
class TransformMatrixDecorator : public IPrimitive {
public:
    /// @brief Constructs a matrix decorator.
    /// @param inner Primitive to wrap.
    /// @param matrix Shear factors
    TransformMatrixDecorator(PrimitivePtr inner, Mat4 matrix);
    void configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;   ///< Wrapped primitive.
    Mat4 _transformMatrix; ///< Matrix local->world
    Mat4 _invMatrix;       ///< Inverse matrix.
};

#endif /* DECORATORS_HPP_ */
