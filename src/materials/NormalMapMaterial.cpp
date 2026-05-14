/*
 * Project: Raytracer
 * File name: NormalMapMaterial.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Normal map material implementation that perturbs surface normals using a texture.
 */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdint>
#include "NormalMapMaterial.hpp"
#include "Common.hpp"

NormalMapMaterial::NormalMapMaterial(std::shared_ptr<IMaterial> baseMaterial, const std::string &normalMapPath)
    : _baseMaterial(std::move(baseMaterial)) {
    try {
        loadPPM(normalMapPath);
    } catch (const std::runtime_error &e) {
        if (std::string(e.what()).find("Cannot open texture file") != std::string::npos) {
            _pixels.clear();
            _width = 0;
            _height = 0;
        } else {
            throw;
        }
    }
}

ScatterResult NormalMapMaterial::scatter(const Ray &ray, const HitRecord &hit) const {
    ScatterResult result = _baseMaterial->scatter(ray, hit);

    if (_pixels.empty() || length(hit.tangent) < epsilon || length(hit.bitangent) < epsilon)
        return result;

    int px = static_cast<int>(hit.UV.first * _width) % _width;
    if (px < 0)
        px += _width;
    int py = static_cast<int>(hit.UV.second * _height) % _height;
    if (py < 0)
        py += _height;

    Color texColor = _pixels[py * _width + px];
    Vec3 nTangent = normalize(2.0 * Vec3(texColor.x, texColor.y, texColor.z) - Vec3(1.0, 1.0, 1.0));

    Vec3 nWorld = normalize(hit.tangent * nTangent.x + hit.bitangent * nTangent.y + hit.normal * nTangent.z);
    result.modifiedNormal = nWorld;

    return result;
}

void NormalMapMaterial::loadPPM(const std::string &filePath) {
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

    long long size = static_cast<long long>(_width) * _height;

    _pixels.resize(size);

    if (magic == "P3") {
        for (long long i = 0; i < size; i++) {
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
        std::vector<uint8_t> data(size * 3);
        file.read(reinterpret_cast<char*>(data.data()), data.size());
        if (file.gcount() != static_cast<std::streamsize>(data.size()))
            throw std::runtime_error("Incomplete PPM binary data in: " + filePath);
        for (long long i = 0; i < size; i++) {
            long long idx = i * 3;
            _pixels[i] = Color{
                static_cast<double>(data[idx]) / maxVal,
                static_cast<double>(data[idx + 1]) / maxVal,
                static_cast<double>(data[idx + 2]) / maxVal
            };
        }
    }
}
