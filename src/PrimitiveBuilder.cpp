/*
 * Project: Raytracer
 * File name: PrimitiveBuilder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Builder pattern assembling a configured and decorated IPrimitive
 *                   from a factory, optional transforms (translation, rotation, scale), and a material.
 */

#include "PrimitiveBuilder.hpp"
#include "Decorators.hpp"
#include <stdexcept>

PrimitiveBuilder &PrimitiveBuilder::setType(const std::string &type) {
    _type = type;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setMaterial(std::shared_ptr<IMaterial> material) {
    _material = material;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setTranslation(const Vec3 &t) {
    _translation = t;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setRotation(const Vec3 &r) {
    _rotation = r;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setScale(const Vec3 &s) {
    _scale = s;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setShear(const ShearFactors &s) {
    _shear = s;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setTransformMatrix(const Mat4 &m) {
    _transformMatrix = m;
    return *this;
}

PrimitiveBuilder &PrimitiveBuilder::setParams(const std::unordered_map<std::string, double> &params) {
    _params = params;
    return *this;
}

void PrimitiveBuilder::reset() {
    _type.clear();
    _material.reset();
    _translation.reset();
    _rotation.reset();
    _scale.reset();
    _shear.reset();
    _transformMatrix.reset();
    _params.clear();
}

PrimitivePtr PrimitiveBuilder::build() {
    if (_type.empty())
        throw std::runtime_error("PrimitiveBuilder::build() called without setType()");

    auto optBase = _factory.create(_type);
    if (!optBase.has_value())
        throw std::runtime_error("PrimitiveBuilder::build(): unknown primitive type '" + _type + "'");

    PrimitivePtr primitive = std::move(*optBase);
    primitive->configure(_params, _material);

    if (_scale)
        primitive = PrimitivePtr(
            new ScaleDecorator(std::move(primitive), _scale.value()),
            &defaultDestroy<ScaleDecorator>
        );
    if (_shear)
        primitive = PrimitivePtr(
            new ShearDecorator(std::move(primitive),
                _shear->sxy, _shear->sxz, _shear->syx,
                _shear->syz, _shear->szx, _shear->szy),
            &defaultDestroy<ShearDecorator>
        );
    if (_rotation)
        primitive = PrimitivePtr(
            new RotationDecorator(std::move(primitive), _rotation.value()),
            &defaultDestroy<RotationDecorator>
        );
    if (_translation)
        primitive = PrimitivePtr(
            new TranslationDecorator(std::move(primitive), _translation.value()),
            &defaultDestroy<TranslationDecorator>
        );
    if (_transformMatrix)
        primitive = PrimitivePtr(
            new TransformMatrixDecorator(std::move(primitive), _transformMatrix.value()),
            &defaultDestroy<RotationDecorator>
        );
    
    return primitive;
}
