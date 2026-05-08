/*
 * Project: Raytracer
 * File name: TexturedMaterial.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Textured material implementation; loads PPM image and samples color using UV.
 */

#include "TexturedMaterial.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

TexturedMaterial::TexturedMaterial(const std::string &filePath) {
    try {
        loadPPM(filePath);
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()).find("Cannot open texture file") != std::string::npos) {
            _pixels.clear();
            _width = 0;
            _height = 0;
        } else {
            throw;
        }
    }
}

ScatterResult TexturedMaterial::scatter(const Ray &/*ray*/, const HitRecord &hit) const {
    if (_pixels.empty())
        return ScatterResult{Color{1.0, 0.0, 1.0}, std::nullopt};

    int px = static_cast<int>(hit.UV.first * _width) % _width;
    int py = static_cast<int>(hit.UV.second * _height) % _height;
    if (px < 0) px += _width;
    if (py < 0) py += _height;

    Color color = _pixels[py * _width + px];
    return ScatterResult{color, std::nullopt};
}

void TexturedMaterial::loadPPM(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open texture file: " + filePath);

    auto skipComments = [&]() {
        std::string line;
        while ((file >> std::ws).peek() == '#')
            std::getline(file, line);
    };

    skipComments();
    std::string magic;
    file >> magic;
    if (magic != "P3")
        throw std::runtime_error("Only P3 PPM format is supported: " + filePath);

    skipComments();
    file >> _width;
    skipComments();
    file >> _height;
    skipComments();
    int maxVal;
    file >> maxVal;

    _pixels.resize(_width * _height);
    for (int i = 0; i < _width * _height; ++i) {
        int r, g, b;
        if (!(file >> r >> g >> b))
            throw std::runtime_error("Invalid PPM pixel data in: " + filePath);
        _pixels[i] = Color{
            static_cast<double>(r) / maxVal,
            static_cast<double>(g) / maxVal,
            static_cast<double>(b) / maxVal
        };
    }
}
