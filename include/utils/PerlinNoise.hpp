/*
 * Project: Raytracer
 * File name: PerlinNoise.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Perlin noise utility class providing coherent pseudo-random 3D noise
 *                   and fractal Brownian motion for procedural texture generation.
 */

#ifndef PERLINNOISE_HPP_
    #define PERLINNOISE_HPP_
    #include "Vec3.hpp"

constexpr int NB_PERM = 512;
constexpr int MID_NB_PERM = 256;
constexpr int NB_GRADIENTS = 12;

/// @brief Utility class generating coherent pseudo-random 3D Perlin noise.
/// @details Uses a permutation table of 512 entries (0–255 shuffled and duplicated)
/// and 12 gradient vectors on the edges of a unit cube.
/// The noise function produces smooth, continuous values in [0, 1].
/// Fractal Brownian Motion (fBm) is available via fractal().
class PerlinNoise {
public:
    /// @brief Constructs a PerlinNoise instance with the given random seed.
    /// @param seed Seed for the random permutation shuffle. Default is 0.
    explicit PerlinNoise(unsigned int seed = 0);

    /// @brief Evaluates Perlin noise at a 3D point.
    /// @param p The 3D point to evaluate.
    /// @return Noise value in [0, 1].
    double noise(const Vec3 &p) const;

    /// @brief Evaluates fractal Brownian motion by summing noise octaves.
    /// @details Each octave doubles the frequency and halves the amplitude.
    /// The result is normalized by the sum of amplitudes.
    /// @param p The 3D point to evaluate.
    /// @param octaves Number of noise octaves to sum.
    /// @return Noise value in [0, 1].
    double fractal(const Vec3 &p, int octaves) const;

private:
    /// @brief Smooth quintic fade curve f(t) = 6t⁵ − 15t⁴ + 10t³ (C² continuity).
    static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }

    /// @brief Linear interpolation between a and b by factor t.
    static double lerp(double a, double b, double t) { return a + t * (b - a); }

    /// @brief Dot product between the gradient at hash index and the offset vector.
    double dotGradient(int hash, double dx, double dy, double dz) const { return dot(_gradients[hash % NB_GRADIENTS], Vec3(dx, dy, dz)); }

    int _perm[NB_PERM];            ///< Permutation table: 0–255 shuffled, duplicated to 512 entries.
    Vec3 _gradients[NB_GRADIENTS]; ///< 12 gradient vectors on the edges of a unit cube.
};

#endif /* PERLINNOISE_HPP_ */
