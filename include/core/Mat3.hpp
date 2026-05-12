/*
 * Project: Raytracer
 * File name: Mat3.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: 3x3 matrix implementation for rotations and transformations with
 *                   rotation matrix factories and utility functions.
 */

#ifndef MAT3_HPP_
    #define MAT3_HPP_
    #include <cmath>
    #include "Vec3.hpp"

/// @brief 3x3 matrix for rotations and transformations.
struct Mat3 {
    double m[3][3] = {}; ///< Matrix elements in row-major order [row][column].

    /// @brief Matrix-vector multiplication.
    /// @param v Vector to multiply.
    /// @return Transformed vector.
    Vec3 operator*(const Vec3 &v) const {
        return Vec3(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        );
    }

    /// @brief Matrix-matrix multiplication.
    /// @param o Other matrix.
    /// @return Result matrix.
    Mat3 operator*(const Mat3 &o) const {
        Mat3 r{};
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    /// @brief Returns the transpose of this matrix.
    /// @return Transposed matrix.
    Mat3 transpose() const {
        return Mat3{{
            {m[0][0], m[1][0], m[2][0]},
            {m[0][1], m[1][1], m[2][1]},
            {m[0][2], m[1][2], m[2][2]}
        }};
    }

    Mat3 inverse() const {
        double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                     m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                     m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        if (std::abs(det) < 1e-9)
            throw std::runtime_error("Matrix is singular and cannot be inverted.");
        
        double invDet = 1.0 / det;

        return Mat3{{
            {
                (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet,
                (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet,
                (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet
            },
            {
                (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet,
                (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet,
                (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet
            },
            {
                (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet,
                (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet,
                (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet
            }
        }};
    }
};

/// @brief Creates a rotation matrix around the X-axis.
/// @param theta Rotation angle in radians.
/// @return Rotation matrix.
inline Mat3 rotateX(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {1, 0, 0},
        {0, c, -s},
        {0, s, c}
    }};
}

/// @brief Creates a rotation matrix around the Y-axis.
/// @param theta Rotation angle in radians.
/// @return Rotation matrix.
inline Mat3 rotateY(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {c, 0, s},
        {0, 1, 0},
        {-s, 0, c}
    }};
}

/// @brief Creates a rotation matrix around the Z-axis.
/// @param theta Rotation angle in radians.
/// @return Rotation matrix.
inline Mat3 rotateZ(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return Mat3{{
        {c, -s, 0},
        {s, c, 0},
        {0, 0, 1}
    }};
}

/// @brief Creates a shear matrix.
/// @param sxy Shear X by Y
/// @param sxz Shear X by Z
/// @param syx Shear Y by X
/// @param syz Shear Y by Z
/// @param szx Shear Z by X
/// @param szy Shear Z by Y
/// @return Shear matrix.
inline Mat3 shearMatrix(double sxy, double sxz, double syx, double syz, double szx, double szy) {
    return Mat3{{
        {1, sxy, sxz},
        {syx, 1, syz},
        {szx, szy, 1}
    }};
}

#endif /* MAT3_HPP_ */
