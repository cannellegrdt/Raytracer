/*
 * Project: Raytracer
 * File name: PerlinNoise.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Perlin noise utility class implementation computing coherent pseudo-random 3D noise and fractal Brownian motion.
 */

#include "PerlinNoise.hpp"
#include <random>
#include <algorithm>
#include <cmath>

PerlinNoise::PerlinNoise(unsigned int seed) {
    _gradients[0] = Vec3(1, 1, 0);
    _gradients[1] = Vec3(-1, 1, 0);
    _gradients[2] = Vec3(1, -1, 0);
    _gradients[3] = Vec3(-1, -1, 0);
    _gradients[4] = Vec3(1, 0, 1);
    _gradients[5] = Vec3(-1, 0, 1);
    _gradients[6] = Vec3(1, 0, -1);
    _gradients[7] = Vec3(-1, 0, -1);
    _gradients[8] = Vec3(0, 1, 1);
    _gradients[9] = Vec3(0, -1, 1);
    _gradients[10] = Vec3(0, 1, -1);
    _gradients[11] = Vec3(0, -1, -1);

    for (int i = 0; i < MID_NB_PERM; i++)
        _perm[i] = i;
    std::shuffle(_perm, _perm + MID_NB_PERM, std::mt19937(seed));
    for (int i = 0; i < MID_NB_PERM; i++)
        _perm[i + MID_NB_PERM] = _perm[i];
}

double PerlinNoise::noise(const Vec3 &p) const {
    int ix = (int)std::floor(p.x) & 255;
    int iy = (int)std::floor(p.y) & 255;
    int iz = (int)std::floor(p.z) & 255;

    double dx = p.x - std::floor(p.x);
    double dy = p.y - std::floor(p.y);
    double dz = p.z - std::floor(p.z);

    double fadeX = fade(dx);
    double fadeY = fade(dy);
    double fadeZ = fade(dz);

    int A = _perm[ix] + iy;
    int B = _perm[ix + 1] + iy;
    int AA = _perm[A] + iz;
    int AB = _perm[A + 1] + iz;
    int BA = _perm[B] + iz;
    int BB = _perm[B + 1] + iz;

    double c000 = dotGradient(_perm[AA], dx, dy, dz );
    double c100 = dotGradient(_perm[BA], dx - 1, dy, dz );
    double c010 = dotGradient(_perm[AB], dx, dy - 1, dz );
    double c110 = dotGradient(_perm[BB], dx - 1, dy - 1, dz );
    double c001 = dotGradient(_perm[AA + 1], dx, dy, dz - 1);
    double c101 = dotGradient(_perm[BA + 1], dx - 1, dy, dz - 1);
    double c011 = dotGradient(_perm[AB + 1], dx, dy - 1, dz - 1);
    double c111 = dotGradient(_perm[BB + 1], dx - 1, dy - 1, dz - 1);

    double x0 = lerp(c000, c100, fadeX);
    double x1 = lerp(c010, c110, fadeX);
    double x2 = lerp(c001, c101, fadeX);
    double x3 = lerp(c011, c111, fadeX);

    double y0 = lerp(x0, x1, fadeY);
    double y1 = lerp(x2, x3, fadeY);

    return (lerp(y0, y1, fadeZ) + 1.0) / 2.0;
}

double PerlinNoise::fractal(const Vec3 &p, int octaves) const {
    if (octaves <= 0)
        return 0.0;
    
    double result = 0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxValue = 0;

    for (int i = 0; i < octaves; i++) {
        result += noise(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude /= 2.0;
        frequency *= 2.0;
    }
    return result / maxValue;
}