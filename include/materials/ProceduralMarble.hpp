/*
 * Project: Raytracer
 * File name: ProceduralMarble.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: ...
 */

#ifndef PROCEDURALMARBLE_HPP_
    #define PROCEDURALMARBLE_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"
    #include "PerlinNoise.hpp"

class ProceduralMarble : public IMaterial {
public:
    explicit ProceduralMarble(const Color &colorA, const Color &colorB, double scale, double turbulence, int octaves) : _colors(colorA, colorB), _scale(scale), _turbulence(turbulence), _octaves(octaves) {}

    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    std::pair<Color, Color> _colors;  ///< Pair of colors (colorA, colorB) for the marble pattern.
    double _scale;                    ///< Scale factor determining the size of each marble cell.
    double _turbulence;               ///< ...
    int _octaves;                     ///< ...
    PerlinNoise _noise;               ///< ...
};

#endif /* PROCEDURALMARBLE_HPP_ */
