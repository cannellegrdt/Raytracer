/*
 * Project: Raytracer
 * File name: Vec3.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Vec3 struct with arithmetic operators, dot, cross, length, normalize.
 */

#ifndef VEC3_HPP_
    #define VEC3_HPP_
    #include <cmath>
    #include <stdexcept>
    #include "Common.hpp"

/// @brief 3D vector with x, y, z components.
struct Vec3 {
    double x, y, z;

    /// @brief Constructs a Vec3 with optional components.
    /// @param x X component (default 0)
    /// @param y Y component (default 0)
    /// @param z Z component (default 0)
    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    /// @brief Addition operator.
    /// @param v Vector to add.
    /// @return New Vec3 with summed components.
    Vec3 operator+(const Vec3 &v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    /// @brief Subtraction operator.
    /// @param v Vector to subtract.
    /// @return New Vec3 with difference components.
    Vec3 operator-(const Vec3 &v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    /// @brief Component-wise multiplication operator.
    /// @param v Vector to multiply.
    /// @return New Vec3 with multiplied components.
    Vec3 operator*(const Vec3 &v) const {
        return Vec3(x * v.x, y * v.y, z * v.z);
    }

    /// @brief Component-wise division operator.
    /// @param v Vector to divide by.
    /// @return New Vec3 with divided components (zeros on divide-by-zero).
    Vec3 operator/(const Vec3 &v) const {
        Vec3 newVec = Vec3(0, 0, 0);
        if (v.x != 0) newVec.x = x / v.x;
        if (v.y != 0) newVec.y = y / v.y;
        if (v.z != 0) newVec.z = z / v.z;
        return newVec;
    }

    /// @brief Index operator for mutable access.
    /// @param i Index (0=x, 1=y, 2=z).
    /// @return Reference to component.
    double& operator[](int i) {
        if (i < 0 || i >= 3) throw std::out_of_range("Vec3 index out of range");
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    /// @brief Index operator for const access.
    /// @param i Index (0=x, 1=y, 2=z).
    /// @return Const reference to component.
    const double& operator[](int i) const {
        if (i < 0 || i >= 3) throw std::out_of_range("Vec3 index out of range");
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    /// @brief Negation operator.
    /// @return New Vec3 with negated components.
    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    /// @brief Addition assignment operator.
    /// @param v Vector to add.
    /// @return Reference to this vector.
    Vec3 &operator+=(const Vec3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    /// @brief Subtraction assignment operator.
    /// @param v Vector to subtract.
    /// @return Reference to this vector.
    Vec3 &operator-=(const Vec3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    /// @brief Component-wise multiplication assignment operator.
    /// @param v Vector to multiply.
    /// @return Reference to this vector.
    Vec3 &operator*=(const Vec3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    /// @brief Component-wise division assignment operator.
    /// @param v Vector to divide by.
    /// @return Reference to this vector.
    Vec3 &operator/=(const Vec3 &v) {
        if (v.x != 0) x /= v.x; else x = 0;
        if (v.y != 0) y /= v.y; else y = 0;
        if (v.z != 0) z /= v.z; else z = 0;
        return *this;
    }
};

/// @brief Addition operator: scalar + vector.
/// @param value Scalar value.
/// @param v Vector.
/// @return New Vec3 with scalar added to each component.
inline Vec3 operator+(double value, const Vec3 &v) {
    return Vec3(v.x + value, v.y + value, v.z + value);
}

/// @brief Subtraction operator: scalar - vector.
/// @param value Scalar value.
/// @param v Vector.
/// @return New Vec3 with each component subtracted from scalar.
inline Vec3 operator-(double value, const Vec3 &v) {
    return Vec3(value - v.x, value - v.y, value - v.z);
}

/// @brief Multiplication operator: scalar * vector.
/// @param value Scalar value.
/// @param v Vector.
/// @return New Vec3 with each component multiplied by scalar.
inline Vec3 operator*(double value, const Vec3 &v) {
    return Vec3(v.x * value, v.y * value, v.z * value);
}

/// @brief Addition operator: vector + scalar.
/// @param v Vector.
/// @param value Scalar value.
/// @return New Vec3 with scalar added to each component.
inline Vec3 operator+(const Vec3 &v, double value) {
    return Vec3(v.x + value, v.y + value, v.z + value);
}

/// @brief Subtraction operator: vector - scalar.
/// @param v Vector.
/// @param value Scalar value.
/// @return New Vec3 with scalar subtracted from each component.
inline Vec3 operator-(const Vec3 &v, double value) {
    return Vec3(v.x - value, v.y - value, v.z - value);
}

/// @brief Multiplication operator: vector * scalar.
/// @param v Vector.
/// @param value Scalar value.
/// @return New Vec3 with each component multiplied by scalar.
inline Vec3 operator*(const Vec3 &v, double value) {
    return Vec3(v.x * value, v.y * value, v.z * value);
}

/// @brief Division operator: vector / scalar.
/// @param v Vector.
/// @param value Scalar value.
/// @return New Vec3 with each component divided by scalar.
inline Vec3 operator/(const Vec3& v, double value) {
    return Vec3(v.x / value, v.y / value, v.z / value);
}

/// @brief Computes the dot product of two vectors.
/// @param v1 First vector.
/// @param v2 Second vector.
/// @return Dot product (scalar).
inline double dot(const Vec3 &v1, const Vec3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/// @brief Computes the cross product of two vectors.
/// @param v1 First vector.
/// @param v2 Second vector.
/// @return Cross product vector.
inline Vec3 cross(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    );
}

/// @brief Computes the length (magnitude) of a vector.
/// @param v Vector.
/// @return Length of the vector.
inline double length(const Vec3 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/// @brief Normalizes a vector to unit length.
/// @param v Vector to normalize.
/// @return Normalized vector (zero vector if length < epsilon).
inline Vec3 normalize(const Vec3 &v) {
    double len = length(v);
    if (len < epsilon) return Vec3(0, 0, 0);
    return v / len;
}

/// @brief Computes the reflection of a vector about a normal.
/// @param i Incident vector (pointing toward the surface).
/// @param n Surface normal (unit vector).
/// @return Reflected vector R = i - 2 * dot(n, i) * n.
inline Vec3 reflect(const Vec3 &i, const Vec3 &n) {
    return i - 2.0 * dot(n, i) * n;
}

#endif /* VEC3_HPP_ */
