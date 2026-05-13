# Rendering pipeline

This document explains exactly how the raytracer transforms a `SceneContext` into a PPM image, from pixel generation down to the lighting equation evaluated for each ray.

---

## Overview

```
SceneContext
    │
    ├─ Camera::generateRay(x, y)      ← one ray per pixel (or sub-pixel sample)
    │
    ├─ Renderer::closestHit(ray)      ← BVH traversal  →  HitRecord
    │
    ├─ IMaterial::scatter(ray, hit)   ← attenuation + optional secondary ray + optional normal
    │
    ├─ for each ILight::getSample()   ← shadow rays / AO rays
    │       ↓
    │   Lambert diffuse  +  Phong specular  +  ambient occlusion
    │
    ├─ recursive traceRay()           ← reflection / refraction / transparency
    │
    └─ PPM output (P6 binary)
```

---

## 1. Entry point: `Renderer::render()`

`render()` collects rendering parameters from `SceneContext`:

| Parameter | Source | Default |
|-----------|--------|---------|
| `width`, `height` | `Camera` | - |
| `samples` | `SceneContext::antialiasing::samples` | `1` |
| `aaType` | `SceneContext::antialiasing::type` | `"uniform"` |
| `threshold` | `SceneContext::antialiasing::threshold` | `0.0` |
| `nbAORays` | `SceneContext::nbAORays` | `16` |

It then triggers lazy BVH construction (`scene.bvh()`) before dispatching to `renderTiles()` or `displayLoop()`.

---

## 2. Parallelism and tiling

The image is cut into **16×16 pixel tiles** (`TILE_SIZE = 16`). Tiles are distributed across CPU threads via **OpenMP** (`#pragma omp parallel for schedule(dynamic, 1)`). Each thread owns its own thread-local RNG (`mt19937`), so no locking is needed for random sampling.

---

## 3. Antialiasing

### Uniform sampling (`type = "uniform"`, `samples = N`)

For each pixel $(x, y)$, $N\times N$ sub-pixel samples are cast in a regular grid:
$\ \ \ \ u = x + \frac{i+0.5}{N}$ with $i\in [0, N)$
$\ \ \ \ v = y + \frac{j+0.5}{N}$ with $j\in [0, N)$

The $N^2$ color values are averaged.

### Adaptive sampling (`type = "adaptive"`, `samples = N`, `threshold = t`)

1. **Pass 1 (1 sample/pixel):** render all pixels at 1 sample.
2. **Edge detection:** for each pixel, compute the maximum channel difference with its 4 neighbours. If `maxDiff > threshold`, the pixel is marked for refinement.
3. **Pass 2 ($N^2$ samples):** re-render only the marked pixels with the uniform $N\times N$ grid and replace the value.

---

## 4. Ray generation

`Camera::generateRay(double x, double y)` maps pixel coordinates to a ray in world space using the camera's precomputed rotation matrix and field of view.

---

## 5. Closest-hit traversal

```cpp
std::optional<HitRecord> Renderer::closestHit(const Ray &ray, const Scene &scene)
```

- **With BVH** (always the case after `render()` calls `scene.bvh()`): the BVH is traversed in $O(log\ n)$. Unbounded primitives (planes, infinite cylinders/cones - those that return `AABB::infinite()`) are always tested directly.
- **Without BVH** (fallback): brute-force $O(n)$ scan keeping the smallest `t > epsilon`.

---

## 6. `traceRay()` - step by step

`traceRay` is the heart of the renderer. `MAX_DEPTH = 10` limits recursion.

```
traceRay(ray, scene, depth, nbAORays):
```

### Step 1 - intersection

```cpp
auto hit = closestHit(ray, scene);
if (!hit || !hit->material)
    return scene.backgroundColor();
```

If nothing is hit (or the hit has no material), the background color is returned.

### Step 2 - material scatter

```cpp
ScatterResult scattered = hit->material->scatter(ray, *hit);
```

`ScatterResult` contains:
- `attenuation` - color multiplier (surface color)
- `scatteredRay` - optional secondary ray (reflection, refraction, transmission)
- `modifiedNormal` - optional perturbed normal (used by `NormalMapMaterial`)

The effective normal used for all lighting calculations is:
```cpp
Vec3 effectiveNormal = scattered.modifiedNormal.value_or(hit->normal);
```

### Step 3 - direct lighting loop

For each light in the scene, a `LightSample` is obtained via `light->getSample(hit->point, hit->normal)`.

#### 3a - Ambient light (AO)

If `sample.isAmbient == true`:

```
for i in 0..nbAORays:
    AODir = random direction in the upper hemisphere (relative to hit->normal)
    AORay = Ray(hit->point + RayBias * hit->normal, AODir)
    if no hit within sample.maxDistance:
        unoccluded++

AOFactor = unoccluded / nbAORays
lightDiffuse += sample.color * AOFactor
```

No shadow ray, no specular. `AOFactor` modulates the ambient contribution: 1.0 = fully open, 0.0 = fully occluded.

#### 3b - Directional / point light (shadow + shading)

**Shadow ray with transmissive support:**

The shadow ray starts at `hit->point + RayBias * hit->normal` (bias avoids self-intersection).
Up to **8 transmissive surfaces** are traversed before concluding the light is blocked:

```
shadowFilter = {1, 1, 1}
sRay = Ray(hit->point + RayBias * normal, sample.direction)
remaining = sample.distance

for si in 0..7:
    blocker = closestHit(sRay, scene)
    if no blocker or blocker.t >= remaining:
        break                            // light is visible
    if blocker.material is null or not transmissive:
        fullyBlocked = true; break       // opaque surface
    s = blocker.material.scatter(sRay, blocker)
    shadowFilter *= s.attenuation        // tint through glass/transparency
    if no s.scatteredRay:
        fullyBlocked = true; break
    remaining -= blocker.t
    sRay = s.scatteredRay

if fullyBlocked: skip this light
```

**Lambert diffuse:**

```cpp
double diffuse = max(0.0, dot(effectiveNormal, sample.direction));
lightDiffuse += sample.color * diffuse * shadowFilter;
```

**Phong specular** (only if `material->getSpecular()` returns a value):

```cpp
Vec3 refl = reflect(-sample.direction, effectiveNormal);
Vec3 viewDir = normalize(-ray.direction);
double angle = max(0.0, dot(refl, viewDir));
double specular = pow(angle, specularParams.shininess);
lightSpecular += specularParams.ks * sample.color * specular * shadowFilter;
```

### Step 4 - combine direct + indirect

```cpp
Color result = scattered.attenuation * lightDiffuse + lightSpecular;

if (depth > 0 && scattered.scatteredRay) {
    Color indirect = traceRay(*scattered.scatteredRay, scene, depth - 1, nbAORays);
    result += scattered.attenuation * indirect;
}
return result;
```

Recursion handles:
- **Mirror reflection** (`Reflection` material): scattered ray is the reflected direction.
- **Transparency** (`Transparency`): scattered ray continues in the same direction.
- **Refraction** (`Refraction`): scattered ray is bent by Snell-Descartes law.

The recursion bottoms out when `depth == 0` or when `scatter()` returns no secondary ray.

---

## 7. Output: PPM P6

`writePPM()` writes a **binary PPM (P6)** file to `output/<scene_stem>.ppm`. Each color component is clamped to $[0, 1]$ and scaled to $[0, 255]$ via `toPPMByte()`.

---

## 8. GUI mode (`--gui`)

When `--gui` is passed, `displayLoop()` runs `renderTiles()` on a separate thread while the main thread drives an **SFML window** at 30 fps. The pixel buffer is shared (atomics guard the stop flag). Closing the window or pressing `Escape` sets the stop flag; the render thread skips remaining tiles. The partial image is still written to the PPM file.

---

## 9. Summary table

| Constant | Value | Role |
|----------|-------|------|
| `MAX_DEPTH` | `10` | Max reflection/refraction recursion depth |
| `TILE_SIZE` | `16` | Tile side length in pixels (OpenMP scheduling unit) |
| `RayBias` | `1e-4` | Ray origin offset to avoid self-intersection |
| `epsilon` | `1e-12` | Minimum `t` for a valid intersection |
| Shadow bounce limit | `8` | Max transmissive surfaces traversed per shadow ray |
| `nbAORays` default | `16` | Ambient occlusion rays when not specified in `.cfg` |
