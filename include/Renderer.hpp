/*
 * Project: Raytracer
 * File name: Renderer.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Ray-per-pixel renderer: iterates over all pixels, traces rays against the scene, and accumulates direct and indirect lighting contributions.
 */

#ifndef RENDERER_HPP_
    #define RENDERER_HPP_
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
    void render(const SceneContext &context);

#ifdef UNIT_TEST
    friend class RendererTestAccessor;
#endif

private:
    /// @brief Traces a ray into the scene.
    /// @param ray The ray to trace.
    /// @param scene The scene to trace against.
    /// @param depth Current recursion depth.
    /// @return Color result from tracing the ray.
    Color traceRay(const Ray &ray, const Scene &scene, int depth) const;

    /// @brief Finds the closest hit along a ray.
    /// @param ray The ray to test.
    /// @param scene The scene to test against.
    /// @return Optional HitRecord of closest intersection.
    static std::optional<HitRecord> closestHit(const Ray &ray, const Scene &scene);
};

#endif /* RENDERER_HPP_ */