/*
 * Project: Raytracer
 * File name: Camera.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Camera defined by position, rotation, and fov; generates rays for each pixel.
 */

#ifndef CAMERA_HPP_
    #define CAMERA_HPP_
    #include "Vec3.hpp"
    #include "Ray.hpp"

/// @brief Camera for raytracing.
/// @details Generates rays from pixel coordinates for rendering.
class Camera {
public:
    /// @brief Constructs a camera.
    /// @param position Camera position in world space.
    /// @param rotation Camera rotation (Euler angles).
    /// @param fov Field of view in degrees.
    /// @param width Image width in pixels.
    /// @param height Image height in pixels.
    Camera(Vec3 position, Vec3 rotation, double fov, int width, int height);

    /// @brief Generates a ray for a pixel.
    /// @param x Pixel x coordinate.
    /// @param y Pixel y coordinate.
    /// @return Ray from camera through the pixel.
    Ray generateRay(double x, double y) const;

    /// @brief Gets the image width.
    /// @return Image width in pixels.
    int getWidth() const { return _width; }

    /// @brief Gets the image height.
    /// @return Image height in pixels.
    int getHeight() const { return _height; }

private:
    Vec3 _position;    ///< Camera position in world space.
    Vec3 _rotation;    ///< Camera rotation (Euler angles).
    double _fov;       ///< Field of view in degrees.
    int _width;        ///< Image width in pixels.
    int _height;       ///< Image height in pixels.
};

#endif /* CAMERA_HPP_ */
