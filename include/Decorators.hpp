/*
 * Project: Raytracer
 * File name: Decorators.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#ifndef DECORATORS_HPP_
    #define DECORATORS_HPP_
    #include "Type.hpp"
    #include "Mat3.hpp"

class TranslationDecorator : public IPrimitive {
public:
    TranslationDecorator(PrimitivePtr inner, const Vec3 &offset);
    void configure(const std::unordered_map<std::string, double> &params, const IMaterial *mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _offset;
};

class RotationDecorator : public IPrimitive {
public:
    RotationDecorator(PrimitivePtr inner, const Vec3 &angles);
    void configure(const std::unordered_map<std::string, double> &params, const IMaterial *mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _angles;
    Vec3 _center;
    Mat3 _rotation;
    Mat3 _invRotation;
};

class ScaleDecorator : public IPrimitive {
public:
    ScaleDecorator(PrimitivePtr inner, const Vec3 &scale);
    void configure(const std::unordered_map<std::string, double> &params, const IMaterial *mat) override;
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _scale;
};

#endif /* DECORATORS_HPP_ */
