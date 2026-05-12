/*
 * Project: Raytracer
 * File name: TexturedMaterial.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Textured material implementation that reads color from an image file
 *                   using UV coordinates for texture mapping, supporting PPM format images.
 */

#ifndef TEXTUREDMATERIAL_HPP_
    #define TEXTUREDMATERIAL_HPP_
    #include "ScatterResult.hpp"
    #include "IMaterial.hpp"
    #include "Color.hpp"
    #include <vector>
    #include <string>

    /// @brief Textured material returning color from an image based on UV coordinates.
    class TexturedMaterial : public IMaterial {
    public:
        /// @brief Constructs a TexturedMaterial from an image file.
        /// @param filePath Path to the image file (PPM format).
        explicit TexturedMaterial(const std::string &filePath);

        /// @brief Computes the scattering of a ray hitting a surface.
        /// @param ray Incoming ray.
        /// @param hit Hit record with intersection details and UV coordinates.
        /// @return ScatterResult containing texture color attenuation with no scattered ray.
        ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

    private:
        /// @brief Loads a PPM image file.
        /// @param filePath Path to the PPM file.
        void loadPPM(const std::string &filePath);

        int _width;                 ///< Width of the texture image in pixels.
        int _height;                ///< Height of the texture image in pixels.
        std::vector<Color> _pixels; ///< Flattened image pixels (row-major order).
    };

#endif /* TEXTUREDMATERIAL_HPP_ */
