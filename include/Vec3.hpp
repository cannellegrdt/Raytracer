/*
 * Project: Raytracer
 * File name: Vec3.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Vec3 struct with arithmetic operators, dot, cross, length, normalize.
 */

#ifndef VEC3_HPP_
    #define VEC3_HPP_
    #include <cmath>
    constexpr double epsilon = 1e-12;

struct Vec3 {
    double x, y, z;

    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 &v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-(const Vec3 &v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    Vec3 operator*(const Vec3 &v) const {
        return Vec3(x * v.x, y * v.y, z * v.z);
    }

    Vec3 operator/(const Vec3 &v) const {
        Vec3 newVec = Vec3(0, 0, 0);
        if (v.x != 0) newVec.x = x / v.x;
        if (v.y != 0) newVec.y = y / v.y;
        if (v.z != 0) newVec.z = z / v.z;
        return newVec;
    }

    double& operator[](int i) {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }
    const double& operator[](int i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3 &operator+=(const Vec3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3 &operator-=(const Vec3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3 &operator*=(const Vec3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    Vec3 &operator/=(const Vec3 &v) {
        if (v.x != 0) x /= v.x; else x = 0;
        if (v.y != 0) y /= v.y; else y = 0;
        if (v.z != 0) z /= v.z; else z = 0;
        return *this;
    }
};

inline Vec3 operator+(double value, const Vec3 &v) {
    return Vec3(v.x + value, v.y + value, v.z + value);
}

inline Vec3 operator-(double value, const Vec3 &v) {
    return Vec3(value - v.x, value - v.y, value - v.z);
}

inline Vec3 operator*(double value, const Vec3 &v) {
    return Vec3(v.x * value, v.y * value, v.z * value);
}

inline Vec3 operator+(const Vec3 &v, double value) {
    return Vec3(v.x + value, v.y + value, v.z + value);
}

inline Vec3 operator-(const Vec3 &v, double value) {
    return Vec3(v.x - value, v.y - value, v.z - value);
}

inline Vec3 operator*(const Vec3 &v, double value) {
    return Vec3(v.x * value, v.y * value, v.z * value);
}

inline Vec3 operator/(const Vec3& v, double value) {
    return Vec3(v.x / value, v.y / value, v.z / value);
}

inline double dot(const Vec3 &v1, const Vec3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vec3 cross(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    );
}

inline double length(const Vec3 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vec3 normalize(const Vec3 &v) {
    double len = length(v);
    if (len < epsilon) return Vec3(0, 0, 0);
    return v / len;
}

#endif /* VEC3_HPP_ */
