/*
 * Project: Raytracer
 * File name: Decorators.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#include "Decorators.hpp"

TranslationDecorator::TranslationDecorator(PrimitivePtr inner, const Vec3 &offset)
    : _inner(std::move(inner)), _offset(offset) {}

void TranslationDecorator::configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) {
    _inner->configure(params, std::move(mat));
}

std::optional<HitRecord> TranslationDecorator::intersect(const Ray &ray) const {
    Ray localRay = {ray.origin - _offset, ray.direction};
    std::optional<HitRecord> hit = _inner->intersect(localRay);
    if (hit == std::nullopt) return std::nullopt;

    hit->point += _offset;
    return hit;
}

RotationDecorator::RotationDecorator(PrimitivePtr inner, const Vec3 &angles)
    : _inner(std::move(inner)), _angles(angles) {
    _rotation = rotateX(angles.x) * rotateY(angles.y) * rotateZ(angles.z);
    _invRotation = _rotation.transpose();
}

void RotationDecorator::configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) {
    _inner->configure(params, std::move(mat));
}

std::optional<HitRecord> RotationDecorator::intersect(const Ray &ray) const {
    Ray localRay = {_invRotation * ray.origin, _invRotation * ray.direction};

    std::optional<HitRecord> hit = _inner->intersect(localRay);
    if (hit == std::nullopt) return std::nullopt;

    hit->point = _rotation * hit->point;
    hit->normal = normalize(_rotation * hit->normal);
    return hit;
}

ScaleDecorator::ScaleDecorator(PrimitivePtr inner, const Vec3 &scale)
    : _inner(std::move(inner)), _scale(scale) {
    if (scale.x == 0.0 || scale.y == 0.0 || scale.z == 0.0)
        throw std::invalid_argument("ScaleDecorator: scale components must be non-zero");
}

void ScaleDecorator::configure(const std::unordered_map<std::string, double> &params, std::shared_ptr<IMaterial> mat) {
    _inner->configure(params, std::move(mat));
}

std::optional<HitRecord> ScaleDecorator::intersect(const Ray &ray) const {
    Ray localRay = {
        {ray.origin.x / _scale.x, ray.origin.y / _scale.y, ray.origin.z / _scale.z},
        {ray.direction.x / _scale.x, ray.direction.y / _scale.y, ray.direction.z / _scale.z}
    };

    std::optional<HitRecord> hit = _inner->intersect(localRay);
    if (hit == std::nullopt) return std::nullopt;

    Vec3 worldPoint = {hit->point.x * _scale.x, hit->point.y * _scale.y, hit->point.z * _scale.z};
    Vec3 rawNormal = {hit->normal.x / _scale.x, hit->normal.y / _scale.y, hit->normal.z / _scale.z};
    Vec3 worldNormal = normalize(rawNormal);

    return HitRecord{hit->t, worldPoint, worldNormal, hit->material, hit->frontFace};
}

