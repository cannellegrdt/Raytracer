/*
 * Project: Raytracer
 * File name: SpecularParams.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Specular reflection parameters structure for Phong shading model,
 *                   defining highlight color and shininess exponent.
 */

#ifndef SPECULARPARAMS_HPP_
    #define SPECULARPARAMS_HPP_
    #include "Vec3.hpp"

    /// @brief Specular reflection parameters for Phong shading.
    struct SpecularParams {
        Vec3 ks;          ///< Specular color (highlight color, typically white or material-specific).
        double shininess; ///< Shininess exponent (α). Higher values = smaller, sharper highlights.
    };

#endif /* SPECULARPARAMS_HPP_ */
