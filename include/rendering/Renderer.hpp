/*
 * Project: Raytracer
 * File name: Renderer.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ray-per-pixel renderer implementation that iterates over all pixels,
 *                   traces rays against the scene, and accumulates direct and indirect
 *                   lighting contributions for final image generation.
 */

#ifndef RENDERER_HPP_
    #define RENDERER_HPP_
    #include <atomic>
    #include <string>
    #include <vector>
    #include "Color.hpp"
    #include "SceneContext.hpp"

#ifdef UNIT_TEST
class RendererTestAccessor;
#endif

/// @brief Ray-tracing renderer.
/// @details Iterates over all pixels, traces rays against the scene, and
/// accumulates direct and indirect lighting contributions.
class Renderer {
public:
    /// @brief Renders the scene.
    /// @param context Scene context containing all rendering information.
    /// @param outputPath Path for the output PPM file.
    /// @param display If true, opens an SFML window for live preview during rendering.
    /// @param shouldStop If set, render tiles will be skipped when this flag is true.
    void render(const SceneContext &context, const std::string &outputPath, bool display = false,
        const std::atomic<bool> *shouldStop = nullptr);

#ifdef UNIT_TEST
    friend class RendererTestAccessor;
#endif

private:
    /// @brief Parameters for rendering tiles.
    struct RenderParams {
        int width;             ///< Image width in pixels.
        int height;            ///< Image height in pixels.
        int samples;           ///< Number of samples per pixel.
        std::string aaType;    ///< Anti-aliasing type (e.g., "uniform", "adaptive").
        double threshold;      ///< Edge detection threshold for adaptive anti-aliasing.
        int nbAORays;          ///< Number of ambient occlusion rays.
    };

    /// @brief Renders image tiles.
    /// @param context Scene context to render.
    /// @param pixelBuffer Buffer to store rendered pixel colors.
    /// @param params Rendering parameters.
    /// @param shouldStop Pointer to atomic boolean for early termination.
    void renderTiles(const SceneContext &context, std::vector<Color> &pixelBuffer,
        const RenderParams &params, const std::atomic<bool> *shouldStop) const;
    
    /// @brief Writes the rendered image to a PPM file.
    /// @param outputPath Path for the output PPM file.
    /// @param pixelBuffer Buffer containing rendered pixel colors.
    /// @param width Image width in pixels.
    /// @param height Image height in pixels.
    void writePPM(const std::string &outputPath, const std::vector<Color> &pixelBuffer,
        int width, int height) const;
    
    /// @brief Runs the display loop for live preview.
    /// @param pixelBuffer Buffer containing rendered pixel colors.
    /// @param params Rendering parameters.
    /// @param context Scene context being rendered.
    /// @param externalStop Pointer to atomic boolean for early termination.
    void displayLoop(std::vector<Color> &pixelBuffer,
        const RenderParams &params, const SceneContext &context,
        const std::atomic<bool> *externalStop) const;

    /// @brief Traces a ray into the scene.
    /// @param ray Ray to trace.
    /// @param scene Scene to trace against.
    /// @param depth Current recursion depth.
    /// @param nbAORays Number of ambient occlusion rays.
    /// @return Color result of tracing the ray.
    Color traceRay(const Ray &ray, const Scene &scene, int depth, int nbAORays) const;

    /// @brief Finds the closest hit along a ray.
    /// @param ray Ray to trace.
    /// @param scene Scene to trace against.
    /// @return Optional HitRecord containing intersection data if hit occurs.
    static std::optional<HitRecord> closestHit(const Ray &ray, const Scene &scene);
};

#endif /* RENDERER_HPP_ */
