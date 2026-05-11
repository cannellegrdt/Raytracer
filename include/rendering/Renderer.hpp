/*
 * Project: Raytracer
 * File name: Renderer.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ray-per-pixel renderer: iterates over all pixels, traces rays against the scene, and accumulates direct and indirect lighting contributions.
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
    struct RenderParams {
        int width;
        int height;
        int samples;
        std::string aaType;
        double threshold;
        int nbAORays;
    };

    void renderTiles(const SceneContext &context, std::vector<Color> &pixelBuffer,
        const RenderParams &params, const std::atomic<bool> *shouldStop) const;
    void writePPM(const std::string &outputPath, const std::vector<Color> &pixelBuffer,
        int width, int height) const;
    void displayLoop(std::vector<Color> &pixelBuffer,
        const RenderParams &params, const SceneContext &context,
        const std::atomic<bool> *externalStop) const;

    /// @brief Traces a ray into the scene.
    Color traceRay(const Ray &ray, const Scene &scene, int depth, int nbAORays) const;

    /// @brief Finds the closest hit along a ray.
    static std::optional<HitRecord> closestHit(const Ray &ray, const Scene &scene);
};

#endif /* RENDERER_HPP_ */
