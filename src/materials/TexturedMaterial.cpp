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
#include <vector>
#include <cstdint>
#include <cmath>

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
        return ScatterResult{Color{1.0, 0.0, 1.0}, std::nullopt, std::nullopt};

    if (std::isnan(hit.UV.first) || std::isnan(hit.UV.second))
        return ScatterResult{Color{1.0, 0.0, 1.0}, std::nullopt, std::nullopt};

    int px = static_cast<int>(hit.UV.first * _width) % _width;
    int py = static_cast<int>(hit.UV.second * _height) % _height;
    if (px < 0) px += _width;
    if (py < 0) py += _height;

    Color color = _pixels[py * _width + px];
    return ScatterResult{color, std::nullopt, std::nullopt};
}

void TexturedMaterial::loadPPM(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open texture file: " + filePath);

    auto skipComments = [&]() {
        char c;
        while ((file >> std::ws).get(c)) {
            if (c == '#') {
                std::string line;
                std::getline(file, line);
            } else {
                file.unget();
                break;
            }
        }
    };

    skipComments();
    std::string magic;
    file >> magic;
    if (magic != "P3" && magic != "P6")
        throw std::runtime_error("Unsupported PPM format (only P3 and P6): " + filePath);

    skipComments();
    file >> _width;
    skipComments();
    file >> _height;
    skipComments();
    int maxVal;
    file >> maxVal;
    file.get();

    _pixels.resize(static_cast<size_t>(_width) * _height);
    if (magic == "P3") {
        for (size_t i=0; i<_pixels.size(); i++) {
            int r, g, b;
            if (!(file >> r >> g >> b))
                throw std::runtime_error("Invalid PPM pixel data in: " + filePath);
            _pixels[i] = Color{
                static_cast<double>(r) / maxVal,
                static_cast<double>(g) / maxVal,
                static_cast<double>(b) / maxVal
            };
        }
    } else {
        std::vector<uint8_t> data(_pixels.size() * 3);
        file.read(reinterpret_cast<char*>(data.data()), data.size());
        if (file.gcount() != static_cast<std::streamsize>(data.size()))
            throw std::runtime_error("Incomplete PPM binary data in: " + filePath);
        for (size_t i=0; i<_pixels.size(); i++) {
            size_t idx = i * 3;
            _pixels[i] = Color{
                static_cast<double>(data[idx]) / maxVal,
                static_cast<double>(data[idx + 1]) / maxVal,
                static_cast<double>(data[idx + 2]) / maxVal
            };
        }
    }
}
