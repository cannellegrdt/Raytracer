/*
 * Project: Raytracer
 * File name: Mat3.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: 3x3 rotation matrix with rotateX/Y/Z factory functions.
 */

#ifndef MAT3_HPP_
    #define MAT3_HPP_
    #include <cmath>
    #include "Vec3.hpp"

struct Mat3 {
    double m[3][3] = {};

    Vec3 operator*(const Vec3 &v) const {
        return Vec3(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        );
    }

    Mat3 operator*(const Mat3 &o) const {
        Mat3 r{};
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    Mat3 transpose() const {
        return Mat3{{
            {m[0][0], m[1][0], m[2][0]},
            {m[0][1], m[1][1], m[2][1]},
            {m[0][2], m[1][2], m[2][2]}
        }};
    }
};

inline Mat3 rotateX(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {1, 0, 0},
        {0, c, -s},
        {0, s, c}
    }};
}

inline Mat3 rotateY(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {c, 0, s},
        {0, 1, 0},
        {-s, 0, c}
    }};
}

inline Mat3 rotateZ(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {c, -s, 0},
        {s, c, 0},
        {0, 0, 1}
    }};
}

#endif /* MAT3_HPP_ */
