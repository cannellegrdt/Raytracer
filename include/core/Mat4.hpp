/*
 * Project: Raytracer
 * File name: Mat4.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: 4x4 homogeneous matrix for 3D transformations.
 */

#ifndef MAT4_HPP_
    #define MAT4_HPP_

    #include <cmath>
    #include <stdexcept>
    #include "Vec3.hpp"

    /// @brief 4x4 homogeneous matrix for 3D transformations.
    struct Mat4 {
        double m[4][4] = {};

        /// @brief Matrix-matrix multiplication.
        /// @param o Other matrix.
        /// @return Result matrix.
        Mat4 operator*(const Mat4 &o) const {
            Mat4 r{};
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    for (int k = 0; k < 4; k++)
                        r.m[i][j] += m[i][k] * o.m[k][j];
            return r;
        }

        /// @brief Returns the transpose of this matrix.
        /// @return Transposed matrix.
        Mat4 transpose() const {
            return Mat4{{
                {m[0][0], m[1][0], m[2][0], m[3][0]},
                {m[0][1], m[1][1], m[2][1], m[3][1]},
                {m[0][2], m[1][2], m[2][2], m[3][2]},
                {m[0][3], m[1][3], m[2][3], m[3][3]}
            }};
        }

        /// @brief Returns the inverse of this matrix using Gaussian elimination.
        /// @return Inverted matrix.
        Mat4 inverse() const {
            Mat4 inv{};
            double aug[4][8] = {};

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    aug[i][j] = m[i][j];
                    aug[i][j + 4] = (i == j) ? 1.0 : 0.0;
                }
            }

            for (int col = 0; col < 4; col++) {
                int pivot = col;
                for (int row = col + 1; row < 4; row++) {
                    if (std::abs(aug[row][col]) > std::abs(aug[pivot][col]))
                        pivot = row;
                }

                if (std::abs(aug[pivot][col]) < 1e-9)
                    throw std::runtime_error("Matrix is singular and cannot be inverted.");

                if (pivot != col) {
                    for (int j = 0; j < 8; j++) {
                        std::swap(aug[col][j], aug[pivot][j]);
                    }
                }

                double div = aug[col][col];
                for (int j = 0; j < 8; j++)
                    aug[col][j] /= div;

                for (int row = 0; row < 4; row++) {
                    if (row != col && std::abs(aug[row][col]) > 1e-9) {
                        double factor = aug[row][col];
                        for (int j = 0; j < 8; j++)
                            aug[row][j] -= factor * aug[col][j];
                    }
                }
            }

            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    inv.m[i][j] = aug[i][j + 4];

            return inv;
        }
    };

    /// @brief Transforms a point (Vec3 with w=1, translation applied).
    /// @param m Transformation matrix.
    /// @param p Point to transform.
    /// @return Transformed point.
    inline Vec3 transformPoint(const Mat4 &m, const Vec3 &p) {
        return Vec3(
            m.m[0][0]*p.x + m.m[0][1]*p.y + m.m[0][2]*p.z + m.m[0][3],
            m.m[1][0]*p.x + m.m[1][1]*p.y + m.m[1][2]*p.z + m.m[1][3],
            m.m[2][0]*p.x + m.m[2][1]*p.y + m.m[2][2]*p.z + m.m[2][3]
        );
    }

    /// @brief Transforms a direction (Vec3 with w=0, no translation).
    /// @param m Transformation matrix.
    /// @param d Direction to transform.
    /// @return Transformed direction.
    inline Vec3 transformDirection(const Mat4 &m, const Vec3 &d) {
        return Vec3(
            m.m[0][0]*d.x + m.m[0][1]*d.y + m.m[0][2]*d.z,
            m.m[1][0]*d.x + m.m[1][1]*d.y + m.m[1][2]*d.z,
            m.m[2][0]*d.x + m.m[2][1]*d.y + m.m[2][2]*d.z
        );
    }

    /// @brief Creates an identity 4x4 matrix.
    /// @return Identity matrix.
    inline Mat4 identity4() {
        return Mat4{{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a translation matrix.
    /// @param tx Translation along X.
    /// @param ty Translation along Y.
    /// @param tz Translation along Z.
    /// @return Translation matrix.
    inline Mat4 translate(double tx, double ty, double tz) {
        return Mat4{{
            {1, 0, 0, tx},
            {0, 1, 0, ty},
            {0, 0, 1, tz},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a scaling matrix.
    /// @param sx Scale along X.
    /// @param sy Scale along Y.
    /// @param sz Scale along Z.
    /// @return Scaling matrix.
    inline Mat4 scale(double sx, double sy, double sz) {
        return Mat4{{
            {sx, 0, 0, 0},
            {0, sy, 0, 0},
            {0, 0, sz, 0},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a rotation matrix around the X-axis.
    /// @param theta Rotation angle in radians.
    /// @return Rotation matrix.
    inline Mat4 rotateX4(double theta) {
        double c = std::cos(theta);
        double s = std::sin(theta);
        return Mat4{{
            {1, 0, 0, 0},
            {0, c, -s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a rotation matrix around the Y-axis.
    /// @param theta Rotation angle in radians.
    /// @return Rotation matrix.
    inline Mat4 rotateY4(double theta) {
        double c = std::cos(theta);
        double s = std::sin(theta);
        return Mat4{{
            {c, 0, s, 0},
            {0, 1, 0, 0},
            {-s, 0, c, 0},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a rotation matrix around the Z-axis.
    /// @param theta Rotation angle in radians.
    /// @return Rotation matrix.
    inline Mat4 rotateZ4(double theta) {
        double c = std::cos(theta);
        double s = std::sin(theta);
        return Mat4{{
            {c, -s, 0, 0},
            {s, c, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a look-at view matrix (right-handed, like OpenGL).
    /// @param eye Camera position.
    /// @param center Target position.
    /// @param up World up vector.
    /// @return View matrix.
    inline Mat4 lookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up) {
        Vec3 f = normalize(center - eye);
        Vec3 s = normalize(cross(f, up));
        Vec3 u = cross(s, f);

        return Mat4{{
            {s.x, s.y, s.z, dot(-s, eye)},
            {u.x, u.y, u.z, dot(-u, eye)},
            {-f.x, -f.y, -f.z, dot(f, eye)},
            {0, 0, 0, 1}
        }};
    }

    /// @brief Creates a perspective projection matrix (right-handed, like OpenGL).
    /// @param fovY Field of view in radians.
    /// @param aspect Aspect ratio (width/height).
    /// @param near Near clipping plane.
    /// @param far Far clipping plane.
    /// @return Perspective projection matrix.
    inline Mat4 perspective(double fovY, double aspect, double near, double far) {
        double f = 1.0 / std::tan(fovY / 2.0);
        double rangeInv = 1.0 / (near - far);

        return Mat4{{
            {f / aspect, 0, 0, 0},
            {0, f, 0, 0},
            {0, 0, (near + far) * rangeInv, 2 * near * far * rangeInv},
            {0, 0, -1, 0}
        }};
    }

#endif /* MAT4_HPP_ */
