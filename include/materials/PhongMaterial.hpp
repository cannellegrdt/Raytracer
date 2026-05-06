/*
 * Project: Raytracer
 * File name: PhongMaterial.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Phong material with diffuse and specular components for realistic shading.
 */

#ifndef PHONGMATERIAL_HPP_
    #define PHONGMATERIAL_HPP_
    #include "IMaterial.hpp"
    #include "SpecularParams.hpp"
    #include "Color.hpp"

class PhongMaterial : public IMaterial {
public:
    PhongMaterial(const Color &diffuseColor, const Color &specularColor, double shininess)
        : _color(diffuseColor), _specularParams{specularColor, shininess} {}

    ScatterResult scatter(const Ray &/*ray*/, const HitRecord &/*hit*/) const override {
        return ScatterResult{_color, std::nullopt};
    }

    std::optional<SpecularParams> getSpecular() const override {
        return _specularParams;
    }

private:
    Color _color;
    SpecularParams _specularParams;
};

#endif /* PHONGMATERIAL_HPP_ */
