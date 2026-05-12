# Adding a new scene loader

This guide walks through creating a new scene file format loader. Like lights and materials, loaders are compiled directly into the core binary : they are **not** plugins.

---

## Overview

1. Create a `.hpp` header and `.cpp` source in `include/utils/` and `src/utils/`
2. Implement the `ISceneLoader` interface
3. Register it in `main.cpp` with the file extension it handles
4. Rebuild with `make`

---

## How the loader system works

In `main.cpp`, a `LoaderFactory` maps file extensions to loader creators:

```cpp
PrimitiveFactory primitiveFact;
LoaderFactory    loaderFact;

loaderFact.registerType(".cfg", []() {
    return std::make_unique<LibconfigLoader>();
});

PluginLoader pluginLoader;
pluginLoader.loadAll("./plugins", primitiveFact);

std::string ext = std::filesystem::path(scene).extension().string();
auto loaderOpt  = loaderFact.create(ext);          // picks by extension
SceneContext ctx = (*loaderOpt)->load(scene, primitiveFact);
```

Your loader only needs to produce a valid `SceneContext`. The renderer doesn't care which format was used.

---

## Required interface

Your class must inherit from `ISceneLoader` (`include/interfaces/ISceneLoader.hpp`):

```cpp
class ISceneLoader {
public:
    virtual SceneContext load(const std::string &filePath,
                              PrimitiveFactory &factory) = 0;
    virtual ~ISceneLoader() = default;
};
```

`PrimitiveFactory` is a `Factory<IPrimitive, PrimitivePtr>` already populated with all `.so` plugins by `PluginLoader::loadAll()` before `load()` is called. You use it through `PrimitiveBuilder`.

---

## Step-by-step: create a JSON loader

### 1. Create the header

Create `include/utils/JsonLoader.hpp`:

```cpp
#ifndef JSONLOADER_HPP_
#define JSONLOADER_HPP_
#include <string>
#include <set>
#include "ISceneLoader.hpp"

class JsonLoader : public ISceneLoader {
public:
    SceneContext load(const std::string &filePath,
                      PrimitiveFactory &factory) override;

private:
    SceneContext load(const std::string &filePath,
                      PrimitiveFactory &factory,
                      std::set<std::string> visited);
};

#endif
```

The private overload with `visited` is needed if you want to support imports with circular-reference detection (same pattern as `LibconfigLoader`). Omit it if your format has no imports.

### 2. Create the implementation

Create `src/utils/JsonLoader.cpp`. The skeleton below shows all the parts you must fill:

```cpp
#include "JsonLoader.hpp"
#include "PrimitiveBuilder.hpp"
#include "MaterialBuilder.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
// ... your JSON library include

SceneContext JsonLoader::load(const std::string &filePath,
                               PrimitiveFactory &factory) {
    return load(filePath, factory, {});
}

SceneContext JsonLoader::load(const std::string &filePath,
                               PrimitiveFactory &factory,
                               std::set<std::string> visited) {
    // 1. Circular import guard
    std::string absPath = std::filesystem::canonical(filePath).string();
    if (visited.count(absPath))
        throw std::runtime_error("Circular import: " + absPath);
    visited.insert(absPath);

    // 2. Parse the file (your JSON library here)
    // auto doc = json::parse(filePath);

    // 3. Build the camera
    // Camera cam({px, py, pz}, {rx, ry, rz}, fov, width, height);
    // std::optional<Camera> cameraOpt = cam;

    // 4. Build primitives
    Scene scene;
    PrimitiveBuilder builder(factory);

    // For each primitive in the JSON:
    //   builder.setType("sphere")
    //          .setParams({{"x",0}, {"y",0}, {"z",5}, {"r",1}})
    //          .setMaterial(material)       // see MaterialBuilder below
    //          .setTranslation(...)         // optional
    //          .build();
    //   scene.addPrimitive(builder.build());
    //   builder.reset();

    // 5. Build lights (no builder - construct directly)
    // scene.addLight(std::make_unique<PointLight>(pos, color, intensity));

    // 6. Optional: background color
    // scene.setBackgroundColor({r, g, b});

    // 7. Return
    return SceneContext{std::move(scene), cameraOpt, std::nullopt, std::nullopt};
}
```

### 3. Register in `main.cpp`

Add one `registerType` call alongside the existing `.cfg` entry:

```cpp
loaderFact.registerType(".cfg", []() {
    return std::make_unique<LibconfigLoader>();
});
loaderFact.registerType(".json", []() {          // ← add this
    return std::make_unique<JsonLoader>();
});
```

Add the include at the top:

```cpp
#include "JsonLoader.hpp"
```

### 4. Rebuild

```sh
make re
```

### 5. Use

```sh
./raytracer scenes/my_scene.json
```

---

## Building a `SceneContext` correctly

`SceneContext` has four fields:

```cpp
struct SceneContext {
    Scene scene;                               // required
    std::optional<Camera> camera;              // required (throw if absent)
    std::optional<Supersampling> antialiasing; // optional
    std::optional<int> nbAORays;               // optional
};
```

If `camera` is empty when `render()` is called, it throws. Raise the error yourself with a clear message (as `LibconfigLoader` does) rather than letting the renderer crash.

### Building a `Camera`

```cpp
Camera cam(
    Vec3{px, py, pz},   // position
    Vec3{rx, ry, rz},   // rotation - Euler angles in degrees (same as .cfg)
    fov,                // vertical field of view in degrees
    width,              // image width in pixels
    height              // image height in pixels
);
```

### Building a `Supersampling`

```cpp
Supersampling aa;
aa.samples = 4;         // samples per side → 4×4 sub-pixels
aa.type = "uniform";    // "uniform" or "adaptive"
aa.threshold = 0.01;    // only used for "adaptive"
```

---

## Reusing existing parsers and builders

You do not need to re-implement material or light parsing from scratch. The existing helpers work on `libconfig::Setting` objects, so they are only directly reusable if your format can bridge to libconfig. For a different format, call the constructors directly:

### Materials: call `MaterialBuilder::build()` if you can pass a `libconfig::Setting`

Otherwise, construct materials directly:

```cpp
#include "FlatColor.hpp"
#include "PhongMaterial.hpp"
#include "Reflection.hpp"
// ...

auto mat = std::make_shared<FlatColor>(Color{r, g, b});
auto mat = std::make_shared<PhongMaterial>(diffuse, specular, shininess);
auto mat = std::make_shared<Reflection>(Color{r, g, b});
auto mat = std::make_shared<Refraction>(Color{r, g, b}, ior);
auto mat = std::make_shared<Transparency>(Color{r, g, b});
```

### Primitives: use `PrimitiveBuilder`

```cpp
PrimitiveBuilder builder(factory);

// Parameters must match kPrimFields in PrimitivesParser.cpp:
builder.setType("sphere")
       .setParams({{"x", 0.0}, {"y", 0.0}, {"z", 5.0}, {"r", 1.0}})
       .setMaterial(mat)
       .setTranslation(Vec3{tx, ty, tz})    // optional
       .setRotation(Vec3{rx, ry, rz})       // optional, radians
       .setScale(Vec3{sx, sy, sz})          // optional
       .setShear(ShearFactors{...})         // optional
       .setTransformMatrix(mat4);           // optional, applied last
scene.addPrimitive(builder.build());
builder.reset();
```

The `setParams` keys must exactly match those in `kPrimFields` (see `adding_a_primitive.md`). The `factory` must already contain the `.so` plugin for the requested type : `PluginLoader` handles this before `load()` is called.

For OBJ meshes, use `setFile()` instead of `setParams()`:

```cpp
builder.setType("obj")
       .setFile("path/to/mesh.obj")
       .setMaterial(mat);
scene.addPrimitive(builder.build());
builder.reset();
```

### Lights: construct directly

```cpp
#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"

scene.addLight(std::make_unique<AmbientLight>(Color{r,g,b}, intensity, maxDist));
scene.addLight(std::make_unique<DirectionalLight>(Vec3{dx,dy,dz}, Color{r,g,b}, intensity));
scene.addLight(std::make_unique<PointLight>(Vec3{px,py,pz}, Color{r,g,b}, intensity));
```

---

## Implementing imports (optional)

If your format supports `import`-style scene merging, follow the `LibconfigLoader` pattern:

1. Recursively call `load(subPath, factory, visited)` to get a `SceneContext`
2. Merge its primitives into the current scene (applying optional transforms via `TransformBuilder::applyTransforms()`)
3. Merge its lights
4. Inherit its `camera`, `antialiasing`, and `nbAORays` only if the current file has none

```cpp
#include "TransformBuilder.hpp"

SceneContext importedCtx = load(subPath, factory, visited);

// Inherit camera/settings if not set in the current file
if (!cameraOpt && importedCtx.camera)
    cameraOpt = importedCtx.camera;

// Merge primitives (with optional transform from the import entry)
for (auto &prim : importedCtx.scene.primitives())
    scene.addPrimitive(TransformBuilder::applyTransforms(std::move(prim), importSettings));

// Merge lights
for (auto &light : importedCtx.scene.lights())
    scene.addLight(std::move(light));
```

`TransformBuilder::applyTransforms(primitive, setting)` reads `translation`, `rotation`, `scale`, `shear`, and `transformation_matrix` from a `libconfig::Setting`. If your format is not libconfig, apply transforms manually with `PrimitiveBuilder`.

---

## Tips

- Throw `std::runtime_error` with informative messages for parse errors (file not found, missing field, circular import) - `main.cpp` catches and prints them
- Validate that `camera` is set before returning; throw if absent
- Use `std::filesystem::canonical()` for import path resolution to normalize `../` and symlinks
- The `PrimitiveFactory` is already fully populated when `load()` is called - you do not need to load plugins yourself
- Run `make unit_tests` to verify nothing is broken after your changes
