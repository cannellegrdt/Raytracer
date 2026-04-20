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

class Camera {
public:
    Camera(Vec3 position, Vec3 rotation, double fov, int width, int height);
    Ray generateRay(int x, int y) const;
};

#endif /* CAMERA_HPP_ */
