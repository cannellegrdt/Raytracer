# Adding a new primitive

This guide walks through creating a new primitive plugin for the raytracer. Every primitive is a shared library (`.so`) loaded at runtime.

---

## Overview

1. Create a `.cpp` file in `src/plugins/`
2. Implement the `IPrimitive` interface
3. Export the two required C entry points
4. Register the primitive name and its fields in `PrimitivesParser.cpp`
5. The Makefile compiles it to `plugins/<name>.so` automatically

---

## Step 1: create the plugin source

Create `src/plugins/myprim.cpp`:

```cpp
#include "IPrimitive.hpp"
#include "IMaterial.hpp"
#include "Vec3.hpp"
#include <optional>
#include <unordered_map>

class MyPrim : public IPrimitive {
public:
    MyPrim() = default;
    ~MyPrim() override = default;

    void configure(const std::unordered_map<std::string, double> &params,
        std::shared_ptr<IMaterial> mat) override
    {
        // Read your geometry parameters from the map
        _px = params.at("x");
        _py = params.at("y");
        _pz = params.at("z");
        _size = params.at("s");
        _material = std::move(mat);
    }

    AABB boundingBox() const override {
        Vec3 half(_size, _size, _size);
        Vec3 center(_px, _py, _pz);
        return AABB(center - half, center + half);
    }

    std::optional<HitRecord> intersect(const Ray &ray) const override {
        // Your ray-primitive intersection logic here
        // Return HitRecord on hit, std::nullopt otherwise
        return std::nullopt;
    }

private:
    double _px = 0, _py = 0, _pz = 0, _size = 1;
    std::shared_ptr<IMaterial> _material;
};

extern "C" IPrimitive *create() { return new MyPrim(); }
extern "C" void destroy(IPrimitive *p) { delete p; }
```

---

## Step 2: register in PrimitivesParser

In `src/parser/PrimitivesParser.cpp`, add two entries:

**`kMapTablePrim`** — maps the `.cfg` key (plural) to the factory key (singular, must match the plugin filename without `.cpp`):

```cpp
static const std::unordered_map<std::string, std::string> kMapTablePrim = {
    // ... existing entries ...
    {"myprims", "myprim"},   // .cfg key "myprims", factory key "myprim"
};
```

The key must be the **plural** form. The factory key must match the plugin filename stem (e.g. `src/plugins/myprim.cpp` → `"myprim"`).

**`kPrimFields`** — lists the parameter names that will be extracted from each `.cfg` entry and passed to `configure()`:

```cpp
static const std::unordered_map<std::string, std::vector<std::string>> kPrimFields = {
    // ... existing entries ...
    {"myprim", {"x", "y", "z", "s"}},
};
```

These field names become the keys in the `params` map passed to your `configure()` method.

---

## Step 3: build

```sh
make plugins/myprim.so
```

Or rebuild everything:

```sh
make re
```

---

## Step 4: use in a scene file

```cfg
primitives = {
    myprims = (
        { x = 0.0; y = 0.0; z = 3.0; s = 2.0;
          material = { type = "flat"; color = { r = 0.8; g = 0.4; b = 0.2; }; }; }
    );
};
```

---

## How it works

The parser is **data-driven**. For all primitives except special cases (`obj_meshes`, `groups`), the flow is:

1. The parser iterates over the `primitives` block, reading each section's name (e.g. `"spheres"`)
2. It looks up the name in `kMapTablePrim` to get the factory key (`"sphere"`)
3. It looks up the factory key in `kPrimFields` to get the list of parameter names
4. It extracts those parameters from the `.cfg` entry into a `std::unordered_map<std::string, double>`
5. It calls `builder.setType(key).setParams(params).setMaterial(material).build()`
6. The builder creates the primitive from the factory (loading the `.so` plugin), calls `configure(params, material)` on it, and wraps it with any transforms

You don't need to touch the parsing logic — just add the two map entries.

---

## HitRecord fields

Your `intersect()` must return a `HitRecord` with these fields:

```cpp
struct HitRecord {
    double t;                                  // Distance along ray
    Vec3 point;                                // Intersection point in world space
    Vec3 normal;                               // Surface normal (must face the incoming ray)
    std::shared_ptr<IMaterial> material;       // Material pointer
    bool frontFace;                            // True if ray hit the outside
    std::pair<double, double> UV = {0, 0};     // Texture coordinates [0,1] (optional)
    Vec3 tangent;                              // Tangent for normal mapping (optional)
    Vec3 bitangent;                            // Bitangent for normal mapping (optional)
};
```

**Important:** The normal must always face the incoming ray (i.e. `dot(ray.direction, normal) < 0`). If the computed normal faces away, flip it and set `frontFace = false`.

---

## Transforms

You do **not** need to handle transforms (translation, rotation, scale, shear, matrix) in your plugin. The `PrimitiveBuilder` automatically wraps your primitive in decorator classes. Just implement the primitive in its local coordinate system.

---

## Special cases

- **OBJ meshes** (`plugins/obj.cpp`): use `setFile()` instead of `setParams()`. The file path is read from the `"file"` field in the `.cfg` entry. This is a hardcoded special case in `PrimitivesParser::parse()`.
- **Groups**: handled as nested scene graph nodes, not primitives.

For a standard geometry primitive, you only need `kMapTablePrim` + `kPrimFields`.

---

## Required entry points

Every plugin **must** export these two C symbols:

```cpp
extern "C" IPrimitive *create() { return new MyPrim(); }
extern "C" void destroy(IPrimitive *p) { delete p; }
```

---

## Tips

- Use an epsilon value (around `1e-4`) to avoid self-intersection (shadow acne)
- Provide UV coordinates if you want texture/normal-map support
- Override `boundingBox()` for BVH culling; return `AABB::infinite()` for unbounded primitives
- Test with a simple scene: one instance of your primitive and a flat material
- Run `make unit_tests` to verify nothing is broken
