# Adding a new material

This guide walks through creating a new material type for the raytracer. Unlike primitives, materials are **not** plugins : they are compiled directly into the core binary.

---

## Overview

1. Create a `.hpp` header and `.cpp` source in `include/materials/` and `src/materials/`
2. Implement the `IMaterial` interface
3. Register it in `MaterialBuilder::build()`
4. Rebuild with `make`

---

## Step-by-step: create a "Toon" (cel-shading) material

### 1. Create the header

Create `include/materials/ToonMaterial.hpp`:

```cpp
#ifndef TOONMATERIAL_HPP_
#define TOONMATERIAL_HPP_
#include "IMaterial.hpp"
#include "Color.hpp"

class ToonMaterial : public IMaterial {
public:
    explicit ToonMaterial(const Color &color, int levels = 4)
        : _color(color), _levels(levels) {}

    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    Color _color;
    int _levels;
};

#endif
```

### 2. Create the implementation

Create `src/materials/ToonMaterial.cpp`:

```cpp
#include "ToonMaterial.hpp"
#include "Common.hpp"

ScatterResult ToonMaterial::scatter(const Ray &/*ray*/, const HitRecord &hit) const {
    // Quantize the normal's Y component into discrete bands
    float band = std::floor(dot(hit.normal, Vec3(0, 1, 0)) * _levels) / _levels;
    Color bandedColor = _color * std::clamp(band, 0.1f, 1.0f);
    return ScatterResult{bandedColor, std::nullopt, std::nullopt};
}
```

### 3. Register in MaterialBuilder

In `src/parser/MaterialBuilder.cpp`, add the include:

```cpp
#include "ToonMaterial.hpp"
```

Then add a branch in `MaterialBuilder::build()`:

```cpp
if (type == "toon") {
    int levels = 4;
    if (mat.exists("levels"))
        levels = static_cast<int>(ConfigUtils::toDouble(mat["levels"]));
    return std::make_shared<ToonMaterial>(color, levels);
}
```

### 4. Rebuild

```sh
make re
```

### 5. Use in a scene file

```cfg
material = {
    type = "toon";
    color = { r = 0.9; g = 0.4; b = 0.1; };
    levels = 5;
};
```

---

## Required interface

Your class must inherit from `IMaterial` (`include/interfaces/IMaterial.hpp`):

```cpp
class MyMaterial : public IMaterial {
public:
    // Required: compute scattering
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

    // Optional: return specular parameters for Phong lighting
    std::optional<SpecularParams> getSpecular() const override;

    // Optional: return true if light passes through (affects shadow rays)
    bool isTransmissive() const override;
};
```

### `ScatterResult` fields

```cpp
struct ScatterResult {
    Vec3 attenuation;                   // Color multiplier (how much light is reflected/transmitted)
    std::optional<Ray> scatteredRay;    // New secondary ray (empty = absorbed)
    std::optional<Vec3> modifiedNormal; // Optional modified normal (e.g. for normal mapping)
};
```

**Three common patterns:**

1. **Simple (flat, procedural):** Return attenuation, no scattered ray.
   ```cpp
   return ScatterResult{myColor, std::nullopt, std::nullopt};
   ```

2. **Reflective:** Return attenuation + reflected ray.
   ```cpp
   Vec3 reflected = normalize(dir - 2 * dot(dir, hit.normal) * hit.normal);
   return ScatterResult{_color, Ray{hit.point + bias * hit.normal, reflected}, std::nullopt};
   ```

3. **Refractive:** Return attenuation + refracted ray (Snell-Descartes).
   ```cpp
   double eta = frontFace ? 1.0 / ior : ior;
   Vec3 refracted = refract(normalize(dir), normal, eta);
   return ScatterResult{_color, Ray{point, refracted}, std::nullopt};
   ```

---

## `getSpecular()` - Phong specular highlights

If your material should support Phong specular highlights (light source reflections), override `getSpecular()`:

```cpp
std::optional<SpecularParams> getSpecular() const override {
    return SpecularParams{Vec3{1.0, 1.0, 1.0}, 32.0};
    //                   specular color       shininess exponent
}
```

The renderer uses these parameters to compute specular highlights when a light sample is available. Return `std::nullopt` if the material has no specular component.

---

## `isTransmissive()` - Shadow rays

Override `isTransmissive()` to return `true` if your material allows light to pass through (e.g. glass, transparency). The shadow ray system uses this to accumulate light attenuation through multiple transmissive surfaces instead of stopping at the first hit.

---

## Parameters from the `.cfg` file

`MaterialBuilder::build()` receives a `libconfig::Setting` object. You can read any fields you need:

```cpp
Color color = {
    ConfigUtils::toDouble(mat["color"]["r"]),
    ConfigUtils::toDouble(mat["color"]["g"]),
    ConfigUtils::toDouble(mat["color"]["b"])
};
double myParam = ConfigUtils::toDouble(mat["myField"]);  // if it exists
```

Use `mat.exists("fieldName")` to check for optional fields before reading them.

---

## Tips

- The `scatter()` method is called per-ray-hit during rendering, so keep it fast
- Access the hit point, normal, UV coordinates, and tangent vectors via the `HitRecord`
- Use `RayBias` (defined in `Common.hpp`) when offsetting ray origins to avoid self-intersection
- If you don't need the ray or hit record, name the parameters unnamed to suppress warnings (e.g. `const Ray &/*ray*/`)
- Test with `make unit_tests` and render a simple scene to verify visually
