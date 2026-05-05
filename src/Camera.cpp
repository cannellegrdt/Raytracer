/*
 * Project: Raytracer
 * File name: Camera.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Camera implementation: FOV-based ray generation from pixel coordinates with aspect-ratio correction.
 */

#include <cmath>
#include <stdexcept>
#include <string>
#include "Camera.hpp"
#include "Mat3.hpp"

Camera::Camera(Vec3 position, Vec3 rotation, double fov, int width, int height)
    : _position(position), _rotation(rotation), _fov(fov),
      _width(width), _height(height) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Camera resolution must be positive (got " +
            std::to_string(width) + "x" + std::to_string(height) + ")");
    if (fov <= 0.0 || fov >= 180.0)
        throw std::invalid_argument("Camera FOV must be in (0, 180) degrees (got " +
            std::to_string(fov) + ")");
}

Ray Camera::generateRay(double x, double y) const {
    double toRad = M_PI / 180;
    double halfHeight = tan(_fov / 2 * toRad);
    double halfWidth = halfHeight * (static_cast<double>(_width) / _height);

    double w = (x + 0.5) / _width * 2.0 - 1.0;
    double h = 1 - (y + 0.5) / _height * 2.0;

    double pixelX = w * halfWidth;
    double pixelY = h * halfHeight;

    Vec3 direction = normalize(Vec3(pixelX, pixelY, 1.0));
    Mat3 rotation = rotateZ(_rotation.z * toRad) * rotateY(_rotation.y * toRad) * rotateX(_rotation.x * toRad);

    return Ray{_position, rotation * direction};
}
