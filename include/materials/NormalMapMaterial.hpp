/*
 * Project: Raytracer
 * File name: NormalMapMaterial.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Material that uses a normal map to perturb surface normals for lighting.
 */

#ifndef NORMALMAPMATERIAL_HPP_
    #define NORMALMAPMATERIAL_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"
    #include <vector>
    #include <string>

/// @brief Material that uses a normal map to perturb surface normals for lighting.
class NormalMapMaterial : public IMaterial {
public:
    /// @brief Constructs a NormalMapMaterial.
    /// @param baseMaterial The underlying material (e.g., flat, phong, textured).
    /// @param normalMapPath Path to the normal map image file (PPM format).
    NormalMapMaterial(std::shared_ptr<IMaterial> baseMaterial, const std::string &normalMapPath);

    /// @brief Computes scattering using the base material, then modifies the normal for lighting.
    /// @param ray Incoming ray.
    /// @param hit Hit record with TBN vectors.
    /// @return ScatterResult with attenuation from base material and modified normal if available.
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

    /// @brief Returns specular parameters from the base material.
    std::optional<SpecularParams> getSpecular() const override { return _baseMaterial->getSpecular(); };

private:
    /// @brief Loads a PPM normal map file.
    void loadPPM(const std::string &filePath);

    std::shared_ptr<IMaterial> _baseMaterial; ///< Underlying material.
    int _width = 0;                           ///< Width of the normal map in pixels.
    int _height = 0;                          ///< Height of the normal map in pixels.
    std::vector<Color> _pixels;               ///< Normal map pixels (row-major order).
};

#endif /* NORMALMAPMATERIAL_HPP_ */
