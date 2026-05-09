# Configuration file format (.cfg)

This document describes the full syntax of scene configuration files used by the raytracer.
Files are parsed by **libconfig++** and loaded by `LibconfigLoader`.

---

## Table of contents

1. [File extension & structure](#1-file-extension--structure)
2. [Camera](#2-camera)
3. [Primitives](#3-primitives)
    3.1. [Sphere](#sphere)
    3.2. [Plane](#plane)
    3.3. [Cylinder (infinite)](#cylinder-infinite)
    3.4. [Limited cylinder](#limited-cylinder)
    3.5. [Cone (infinite)](#cone-infinite)
    3.6. [Limited cone](#limited-cone)
    3.7. [Torus](#torus)
    3.8. [Cube](#cube)
    3.9. [Tanglecube](#tanglecube)
    3.10. [Triangle](#triangle)
    3.11. [OBJ Mesh](#obj-mesh)
    3.12. [Mandelbulb (fractals)](#mandelbulb)
    3.13. [Mobius strip](#mobius-strip)
4. [Transforms on primitives](#4-transforms-on-primitives)
5. [Groups (scene graph)](#5-groups-scene-graph)
6. [Materials](#6-materials)
7. [Lights](#7-lights)
8. [Renderer & antialiasing](#8-renderer--antialiasing)
9. [Imports](#9-imports)
10. [Value ranges & rules](#10-value-ranges--rules)
11. [Style guide](#11-style-guide)
12. [Complete example](#12-complete-example)

---

## 1. File extension & structure

- Extension: `.cfg`
- A file must contain at least the `primitives` and `lights` top-level keys.
- `camera` is optional if it is provided by an imported file.

Top-level keys (all optional except `primitives` and `lights`):

```
camera : Camera definition
renderer : Antialiasing settings
primitives : All geometry
lights : Light sources
imports : Other .cfg files to merge into this scene
```

### Syntax quick-reference

| Construct | Syntax |
|-----------|--------|
| Section | `key = { ... };` or `key: { ... };` |
| List | `key = ( item, item );` |
| Key-value | `key = value;` |
| Vec3 / Color | `{ x = 1.0; y = 0.0; z = 0.0; }` / `{ r = 1.0; g = 0.0; b = 0.0; }` |
| Comment | `# comment` (line start) |

---

## 2. Camera

```cfg
camera = {
    resolution = { width = 1920; height = 1080; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 72.0;
};
```

| Field | Type | Description |
|-------|------|-------------|
| `resolution.width` / `.height` | int | Output image size in pixels |
| `position` | Vec3 | Camera origin in world space |
| `rotation` | Vec3 | Camera orientation (pitch / yaw / roll) in degrees |
| `fieldOfView` | double | Vertical field of view in degrees |

---

## 3. Primitives

All primitives live inside the `primitives` group. Each primitive type has its own named list.

```cfg
primitives = {
    spheres = ( ... );
    planes = ( ... );
    cylinders = ( ... );
    limited_cylinders = ( ... );
    cones = ( ... );
    limited_cones = ( ... );
    torus = ( ... );
    cubes = ( ... );
    tanglecubes = ( ... );
    mobius = ( ... );
    obj_meshes = ( ... );
    mandelbulbs = ( ... );
};
```

Any primitive can carry optional [transform fields](#4-transforms-on-primitives) alongside its geometry.

---

### Sphere

```cfg
spheres = (
    {
        x = 0.0; y = 0.0; z = 5.0;
        r = 1.0;
        material = { type = "flat"; color = { r = 1.0; g = 0.2; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center |
| `r` | double | Radius (> 0) |
| `material` | Material | Surface material |

---

### Plane

```cfg
planes = (
    {
        x = 0.0; y = -1.5; z = 0.0;
        nx = 0.0; ny = 1.0; nz = 0.0;
        material = { type = "flat"; color = { r = 0.8; g = 0.8; b = 0.8; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Any point on the plane |
| `nx`, `ny`, `nz` | double | Surface normal (auto-normalized) |
| `material` | Material | Surface material |

---

### Cylinder (infinite)

```cfg
cylinders = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        ax = 0.0; ay = 1.0; az = 0.0;
        r = 0.5;
        material = { type = "flat"; color = { r = 0.2; g = 0.8; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Point on the axis |
| `ax`, `ay`, `az` | double | Axis direction (auto-normalized) |
| `r` | double | Radius (> 0) |
| `material` | Material | Surface material |

---

### Limited cylinder

```cfg
limited_cylinders = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        ax = 0.0; ay = 1.0; az = 0.0;
        r = 0.5; h = 2.0;
        material = { type = "flat"; color = { r = 0.2; g = 0.8; b = 0.2; }; };
    }
);
```

Same as `cylinders` with one extra field:

| Field | Type | Description |
|-------|------|-------------|
| `h` | double | Height (> 0), the cylinder is bounded symmetrically along the axis |

---

### Cone (infinite)

```cfg
cones = (
    {
        x = 0.0; y = 3.0; z = 6.0;
        ax = 0.0; ay = -1.0; az = 0.0;
        angle = 0.5236;
        material = { type = "flat"; color = { r = 1.0; g = 0.6; b = 0.1; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Apex position |
| `ax`, `ay`, `az` | double | Axis direction (auto-normalized) |
| `angle` | double | Half-angle in **radians** - must be in `(0, π/2)` |
| `material` | Material | Surface material |

---

### Limited cone

```cfg
limited_cones = (
    {
        x = 0.0; y = 3.0; z = 6.0;
        ax = 0.0; ay = -1.0; az = 0.0;
        angle = 0.5236;
        h = 2.0;
        material = { type = "flat"; color = { r = 1.0; g = 0.6; b = 0.1; }; };
    }
);
```

Same as `cones` with one extra field:

| Field | Type | Description |
|-------|------|-------------|
| `h` | double | Height (> 0), measured from apex along the axis |

---

### Torus

```cfg
torus = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        ax = 0.0; ay = 1.0; az = 0.0;
        R = 2.0; r = 0.5;
        material = { type = "flat"; color = { r = 0.8; g = 0.4; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center of the torus (center of the tube's circular path) |
| `ax`, `ay`, `az` | double | Axis direction (auto-normalized) |
| `R` | double | Major radius - distance from center to the center of the tube (> 0) |
| `r` | double | Minor radius - radius of the tube (> 0) |
| `material` | Material | Surface material |

**Notes:**
- If `r > R`, the values are automatically swapped to ensure `R >= r`
- The axis defines the orientation of the torus; the tube circles around this axis
- UV coordinates are generated: `u` from the angle around the major radius, `v` from the angle around the tube

---

### Cube

```cfg
cubes = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        s = 2.0;
        material = { type = "flat"; color = { r = 0.8; g = 0.4; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center of the cube |
| `s` | double | Size (side length) of the cube (> 0); the cube extends ±s/2 from center on each axis |
| `material` | Material | Surface material |

**Notes:**
- The cube is axis-aligned; the bounds are `[x-s/2, x+s/2] × [y-s/2, y+s/2] × [z-s/2, z+s/2]`
- UV coordinates are generated: `u = (point.x - min.x) / (max.x - min.x)`, `v = (point.y - min.y) / (max.y - min.y)`
- Normal is computed based on which face was hit (±X, ±Y, or ±Z)
- Tangent `(1,0,0)` and bitangent `(0,1,0)` are provided for normal mapping

---

### TangleCube

```cfg
tanglecubes = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        s = 1.5;
        material = { type = "flat"; color = { r = 0.8; g = 0.4; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center of the tanglecube |
| `s` | double | Scale of the tanglecube (> 0); controls the size of the implicit surface |
| `material` | Material | Surface material |

**Notes:**
- The tanglecube is an implicit surface defined by the equation: `x⁴ - 5x² + y⁴ - 5y² + z⁴ - 5z² + 11.8 = 0`
- The surface is rendered using ray marching with 2000 steps and binary refinement (30 iterations)
- UV coordinates are generated: `u` from `atan2(z, x)` (longitude), `v` from `asin(y/length)` (latitude)
- Tangent and bitangent are computed from the normal and world up vector for normal mapping support
- The bounding sphere has radius 2.5 for ray intersection culling

---

### Triangle

```cfg
triangles = (
    {
        v0x = -1.0; v0y = 0.0; v0z = 0.0;
        v1x = 0.0; v1y = 2.0; v1z = 0.0;
        v2x = 1.0; v2y = 0.0; v2z = 0.0;
        material = { type = "flat"; color = { r = 1.0; g = 0.2; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `v0x`, `v0y`, `v0z` | double | First vertex position |
| `v1x`, `v1y`, `v1z` | double | Second vertex position |
| `v2x`, `v2y`, `v2z` | double | Third vertex position |
| `material` | Material | Surface material |

**Notes:**
- The normal is computed automatically from the cross product of edges `(v1-v0)` and `(v2-v0)`
- The normal direction follows the right-hand rule: counter-clockwise vertex order gives normal pointing toward viewer
- UV coordinates are generated: `u` and `v` from the barycentric coordinates of the hit point
- Tangent and bitangent vectors are provided for normal mapping support
- The triangle supports all transform types: translation, rotation, scale, shear, and transformation_matrix

---

### OBJ Mesh

OBJ meshes are loaded from external `.obj` wavefront files at render time. They are parsed into triangles internally and organized into a BVH for fast intersection.

```cfg
obj_meshes = (
    {
        file = "path/to/mesh.obj";
        material = { type = "flat"; color = { r = 0.8; g = 0.2; b = 0.2; }; };
    }
);
```

#### Required fields

| Field | Type | Description |
|-------|------|-------------|
| `file` | string | Path to the OBJ file (relative to the working directory) |
| `material` | Material | Surface material (same as for other primitives) |

#### Supported transforms

OBJ meshes support all standard transform fields:

- `translation` — moves the mesh in world space
- `rotation` — rotates around X, Y, Z axes (radians)
- `scale` — scales along each axis (no zero components)
- `shear` — shears the mesh along each axis
- `transformation_matrix` — full 4×4 homogeneous matrix (overrides other transforms)

All transforms are applied in the standard order: **Scale → Shear → Rotation → Translation → Transformation matrix**.

#### OBJ format support

- Vertex positions (`v x y z`)
- Vertex normals (`vn nx ny nz`)
- Face definitions (`f v1 v2 v3` or `f v1//n1 v2//n2 v3//n3`)
- Only triangular faces are supported; polygons with > 3 vertices are automatically triangulated
- Negative indices are supported (relative indexing from end of list)
- Vertex UV coordinates (`vt`) are parsed but not used for texturing (UV comes from the primitive's own mapping)

**Notes:**
- The OBJ file is parsed lazily on first intersection call (when `configure` is invoked)
- Face normals are auto-computed from the cross product of edges if vertex normals are absent
- The mesh is wrapped in a BVH (median-split) for O(log n) ray traversal
- The `file` path is relative to the working directory where the raytracer is executed

---

### Mandelbulb

```cfg
mandelbulbs = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        s = 1.0;
        power = 8.0;
        iters = 20;
        bailout = 2.0;
        material = { type = "flat"; color = { r = 1.0; g = 0.2; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center of the Mandelbulb |
| `s` | double | Scale (> 0); controls the overall size |
| `power` | double | Power of the fractal (e.g., 8.0 for classic Mandelbulb) (> 0) |
| `iters` | int | Maximum iterations for the fractal computation (> 0) |
| `bailout` | double | Bailout value for divergence detection (> 0) |
| `material` | Material | Surface material |

**Notes:**
- The Mandelbulb is a 3D fractal (generalization of the Mandelbrot set), rendered using ray marching with a distance estimator.
- The surface is defined implicitly by iterating `z = z^power + c` in spherical coordinates until divergence.
- Ray marching steps along the ray, evaluating the distance to the surface at each point.
- The bounding box is centered at `(x,y,z)` with radius `s * bailout`.
- UV coordinates are generated: `u` from longitude (atan2), `v` from latitude (acos).
- Tangent and bitangent vectors are computed from the normal for normal mapping support.
- Higher `power` values create more complex, spiky shapes; `power = 2` approximates a sphere.
- Tune `iters` for detail vs performance; `bailout` typically 2.0.
- The fractal is bounded, so no infinite marching.

---

### Mobius Strip

```cfg
mobius = (
    {
        x = 0.0; y = 0.0; z = 0.0;
        R = 2.0; w = 0.2;
        material = { type = "flat"; color = { r = 0.8; g = 0.4; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|-------------|
| `x`, `y`, `z` | double | Center of the Mobius strip |
| `R` | double | Major radius - distance from center to the center of the strip (> 0) |
| `w` | double | Half-width of the strip (> 0) |
| `material` | Material | Surface material |

**Notes:**
- The Mobius strip is a non-orientable surface with one continuous side
- The parametric equation is:
  - `x = (R + v * cos(u/2)) * cos(u)`
  - `y = (R + v * cos(u/2)) * sin(u)`
  - `z = v * sin(u/2)`
  - where `u ∈ [0, 2π]` (angle around the strip) and `v ∈ [-w, w]` (width direction)
- Ray intersection uses Newton-Raphson method with multiple starting points for robustness
- UV coordinates are generated: `u = u / (2π)`, `v = (v + w) / (2w)`
- Tangent and bitangent vectors are provided for normal mapping support

---

## 4. Transforms on primitives

Any primitive entry can include one or more transform fields.
They are applied in this fixed order, regardless of the order written in the file:

**Scale → Shear → Rotation → Translation → Transformation matrix**

All transforms are optional and composable.

---

### Translation

```cfg
translation = { x = 1.0; y = 0.0; z = 0.0; };
```

Moves the primitive by the given offset in world space.

---

### Rotation

```cfg
rotation = { x = 0.0; y = 1.5708; z = 0.0; };
```

Rotates around X, Y, Z axes in sequence, angles in **radians**.

---

### Scale

```cfg
scale = { x = 2.0; y = 1.0; z = 2.0; };
```

Scales the primitive along each axis. No component may be zero.

---

### Shear

```cfg
shear = { sxy = 0.3; sxz = 0.0; syx = 0.0; syz = 0.0; szx = 0.0; szy = 0.0; };
```

| Field | Effect |
|-------|--------|
| `sxy` | Shear X proportional to Y |
| `sxz` | Shear X proportional to Z |
| `syx` | Shear Y proportional to X |
| `syz` | Shear Y proportional to Z |
| `szx` | Shear Z proportional to X |
| `szy` | Shear Z proportional to Y |

---

### Transformation matrix

A full 4×4 homogeneous matrix. Overrides all other transforms if both are present (applied last in the chain).

```cfg
transformation_matrix = (
    ( 1.0, 0.0, 0.0, 2.0 ),
    ( 0.0, 1.0, 0.0, 0.0 ),
    ( 0.0, 0.0, 1.0, 0.0 ),
    ( 0.0, 0.0, 0.0, 1.0 )
);
```

Row-major, in homogeneous coordinates (last column = translation).

---

### Full example with transforms

```cfg
spheres = (
    {
        x = 0.0; y = 0.0; z = 0.0; r = 1.0;
        material = { type = "flat"; color = { r = 0.9; g = 0.3; b = 0.1; }; };

        scale = { x = 2.0; y = 1.0; z = 2.0; };
        rotation = { x = 0.0; y = 0.785; z = 0.0; };
        translation = { x = 3.0; y = 1.0; z = 0.0; };
    }
);
```

---

## 5. Groups (scene graph)

A **group** is a node that collects several child primitives (or other groups) under a shared local transform.
Groups are declared with the key `groups` inside any primitives block (top-level or nested).

```cfg
primitives = {
    groups = (
        {
            # Optional transform applied to the whole group
            translation = { x = 5.0; y = 0.0; z = 0.0; };

            # Children block: same syntax as the top-level primitives block
            children: {
                spheres = (
                    {
                        x = 0.0; y = 0.0; z = 0.0; r = 1.0;
                        material = { type = "flat"; color = { r = 0.2; g = 0.4; b = 0.9; }; };
                    }
                );
                planes = ( ... );
                groups = ( ... );   # Nested groups - arbitrary depth
            };
        }
    );
};
```

**How it works**

- The transform fields on a group entry (translation, rotation, scale, shear,
  transformation_matrix) are applied to the group as a whole, in the same order as for
  individual primitives.
- A child's effective world transform = group transform × child's own transform.
- Nesting depth is unlimited: a group can contain other groups.
- A group with no `children` block is valid (empty group, contributes nothing to the render).

---

### Nested group example

```cfg
groups = (
    {
        # Outer group: the whole car, shifted right
        translation = { x = 4.0; y = 0.0; z = 0.0; };
        children: {
            spheres = (
                # Car body (simplified as a sphere)
                { x = 0.0; y = 0.0; z = 0.0; r = 2.0;
                  material = { type = "flat"; color = { r = 0.8; g = 0.1; b = 0.1; }; }; }
            );
            groups = (
                {
                    # Left wheel, rotated relative to the car
                    translation = { x = -1.5; y = -1.0; z = 0.0; };
                    rotation = { x = 1.5708; y = 0.0; z = 0.0; };
                    children: {
                        limited_cylinders = (
                            { x = 0.0; y = 0.0; z = 0.0; ax = 0.0; ay = 1.0; az = 0.0; r = 0.5; h = 0.3;
                              material = { type = "flat"; color = { r = 0.1; g = 0.1; b = 0.1; }; }; }
                        );
                    };
                }
            );
        };
    }
);
```

---

## 6. Materials

Every primitive requires a `material` block:

```cfg
material = { type = "flat"; color = { r = 1.0; g = 0.2; b = 0.2; }; };
```

| `type` | Behaviour |
|--------|-----------|
| `"flat"` | Solid color, no secondary rays |
| `"reflection"` | Mirror reflection; `color` tints the reflected ray |
| `"transparency"` | Transmits the ray through the surface; `color` tints |
| `"refraction"` | Refracts rays according to Snell-Descartes law; `color` tints, `ior` = index of refraction |
| `"phong"` | Diffuse + specular (Phong model); `color` = diffuse color, `specular` = specular color, `shininess` = α |
| `"textured"` | Samples color from a PPM image file using UV coordinates; no secondary rays |
| `"chessboard"` | Procedural checkerboard pattern calculated from 3D hit position; no secondary rays |
| `"marble"` | Procedural marble-like pattern using Perlin noise; `colorA`/`colorB` = vein colors, `scale` = sine frequency, `turbulence` = noise amplitude, `octaves` = detail level; no secondary rays |
| `"normalmap"` | Uses a normal map texture to perturb surface normals for lighting; wraps a base material |
| `"normalmap"` | Uses a normal map texture to perturb surface normals for lighting; wraps a base material |

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | One of `"flat"`, `"reflection"`, `"transparency"`, `"refraction"`, `"phong"`, `"textured"`, `"chessboard"`, `"marble"` |
| `color` | Color | RGB in `[0.0, 1.0]` (diffuse color for phong) |
| `ior` | double | Index of refraction, > 0 (only for `"refraction"`; common values: air ≈ 1.0, water ≈ 1.33, glass ≈ 1.5) |
| `specular` | Color | RGB specular highlight color (only for `"phong"`) |
| `shininess` | double | Shininess exponent > 0 (only for `"phong"`) |
| `texture` | string | Path to a PPM image file (only for `"textured"`) |
| `colorA` | Color | First color (only for `"chessboard"` and `"marble"`) |
| `colorB` | Color | Second color (only for `"chessboard"` and `"marble"`) |
| `scale` | double | Cell size for `"chessboard"` (> 0, default: 1.0); sine frequency for `"marble"` (> 0, default: 1.0) |
| `turbulence` | double | Amplitude of the Perlin noise perturbation (only for `"marble"`; default: 5.0) |
| `octaves` | int | Number of fractal noise octaves (only for `"marble"`; default: 6) |
| `seed` | int | Random seed for the noise permutation table (only for `"marble"`; default: 0) |

### Textured material

The `"textured"` material reads pixel color from a **PPM image** (P3 or P6 format) using the UV coordinates provided by the primitive at the hit point.

```cfg
material = {
    type = "textured";
    texture = "textures/earth.ppm";
};
```

| Field | Type | Description |
|-------|------|-------------|
| `texture` | string | Path to the PPM file (P3 ASCII format, relative to the working directory) |

**Supported format:** P3 ASCII PPM and P6 binary PPM.

**Fallback:** If the file cannot be opened, the material falls back to a magenta color (`1.0, 0.0, 1.0`) so missing textures are immediately visible without crashing.

**UV mapping:** Each primitive exposes UV coordinates at the hit point. The image is tiled (wraps around) when UV values go outside `[0, 1]`.

---

### Procedural checkerboard

The `"chessboard"` material computes colors analytically from the 3D hit position — no texture file or UV mapping needed.

**How it works:**
- The 3D point of intersection is divided by `scale` and floored to integer grid coordinates `(ix, iy, iz)`
- A parity bit is computed: `(ix & 1) ^ (iy & 1) ^ (iz & 1)` (bitwise XOR, safe with negative values)
- If the result is `0`, `colorA` is used; otherwise `colorB`

```cfg
material = {
    type = "chessboard";
    colorA = { r = 1.0; g = 1.0; b = 1.0; };
    colorB = { r = 0.0; g = 0.0; b = 0.0; };
    scale = 2.0;
};
```

| Field | Type | Description |
|-------|------|-------------|
| `colorA` | Color | First checkerboard color (RGB in `[0.0, 1.0]`) |
| `colorB` | Color | Second checkerboard color (RGB in `[0.0, 1.0]`) |
| `scale` | double | Cell size in world units; smaller = more tiles (default: 1.0) |

**Note:** Unlike texture-based checkers, this is a true 3D pattern — it wraps around spheres, cylinders, and any primitive naturally.

---

### Procedural marble

The `"marble"` material produces a veined marble-like appearance by composing Perlin noise with a sine wave — no texture file needed.

**How it works:**
- A fractal Brownian motion (fBm) value is computed at the 3D hit point by summing `octaves` noise layers, each at double the frequency and half the amplitude of the previous
- The fBm value perturbs a sine wave: `sin(scale × hit.z + turbulence × fractal(hit))`
- The sine result is remapped from `[-1, 1]` to `[0, 1]` and used to interpolate between `colorA` and `colorB`

```cfg
material = {
    type = "marble";
    colorA = { r = 0.9; g = 0.85; b = 0.8; };
    colorB = { r = 0.3; g = 0.2; b = 0.15; };
    scale = 3.0;
    turbulence = 5.0;
    octaves = 6;
    seed = 42;
};
```

| Field | Type | Description |
|-------|------|-------------|
| `colorA` | Color | Base color of the marble (RGB in `[0.0, 1.0]`) |
| `colorB` | Color | Vein color (RGB in `[0.0, 1.0]`) |
| `scale` | double | Spatial frequency of the sine wave; higher = finer veins (> 0, default: 1.0) |
| `turbulence` | double | Amplitude of the noise perturbation; higher = more chaotic veins (default: 5.0) |
| `octaves` | int | Number of fractal noise octaves; higher = more surface detail, slower to compute (default: 6) |
| `seed` | int | Random seed for the permutation table; different values produce different vein patterns (default: 0) |

**Note:** `scale` here controls the frequency of the sine wave along the Z axis, not a cell size as in `"chessboard"`. To orient the veins along a different axis, apply a `rotation` transform to the primitive.

---

### Normal map material

The `"normalmap"` material wraps a **base material** and uses a normal map texture to perturb surface normals for lighting calculations. This creates the illusion of surface detail (bumps, grooves) without adding geometric complexity.

**How it works:**
- The base material provides the surface appearance (color, reflection, etc.)
- A normal map texture (PPM P3 or P6 format) encodes perturbed normals in tangent space
- At each hit point, the normal is read from the texture and transformed from tangent space to world space using the TBN matrix
- The perturbed normal affects diffuse and specular lighting calculations

**Requirements:**
- The primitive must provide tangent (`T`) and bitangent (`B`) vectors (spheres, planes, cylinders, and cones all support this)
- The normal map image uses RGB channels to encode XYZ components of the tangent-space normal: `n_tangent = 2 * color - 1`
- Images are typically blue-tinted because the Z component (stored in blue) is dominant

```cfg
material = {
    type = "normalmap";
    normalmap = "textures/normal_brick.ppm";
    base = {
        type = "phong";
        color = { r = 0.8; g = 0.8; b = 0.8; };
        specular = { r = 1.0; g = 1.0; b = 1.0; };
        shininess = 32;
    };
};
```

| Field | Type | Description |
|-------|------|-------------|
| `normalmap` | string | Path to the normal map PPM file (P3 ASCII format) |
| `base` | Material | The underlying material that defines surface properties (required) |

**Supported base materials:** Any material type (`"flat"`, `"phong"`, `"textured"`, `"reflection"`, `"refraction"`, `"chessboard"`, `"marble"`) can be used as the base.

**Fallback:** If the normal map file cannot be opened, the material falls back to using the base material with unmodified normals (no crash).

**Creating normal maps:** Normal maps are typically generated from height maps or 3D models. The RGB values represent:
- Red channel → X component in tangent space (left/right)
- Green channel → Y component in tangent space (up/down)
- Blue channel → Z component in tangent space (surface normal direction, typically ~0.5-1.0)

---

## 7. Lights

```cfg
lights = {
    ambient = (
        { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.2; maxDist = 10.0; }
    );
    directional = (
        {
            direction = { x = -0.5; y = -1.0; z = 0.5; };
            color = { r = 1.0; g = 0.95; b = 0.9; };
            intensity = 0.8;
        }
    );
};
```

### Ambient

| Field | Type | Description |
|-------|------|-------------|
| `color` | Color | Light color |
| `intensity` | double | Multiplier (> 0) |
| `maxDist` | double | *Optional.* Maximum distance for ambient occlusion rays (default: 10.0) |

### Directional

| Field | Type | Description |
|-------|------|-------------|
| `direction` | Vec3 | Direction of incoming light (auto-normalized) |
| `color` | Color | Light color |
| `intensity` | double | Multiplier (> 0) |

### Point

| Field | Type | Description |
|-------|------|-------------|
| `position` | Vec3 | Position of the point light in world space |
| `color` | Color | Light color |
| `intensity` | double | Multiplier (> 0) |

Multiple entries of the same type are allowed.

---

## 8. Renderer & antialiasing

```cfg
renderer = {
    antialiasing = {
        samples = 4;
        type = "uniform";    # "uniform" or "adaptive"
        threshold = 0.01;    # only used when type = "adaptive"
    };
};
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `samples` | int | 1 | Number of samples per pixel (1 = no AA) |
| `type` | string | `"uniform"` | Sampling strategy |
| `threshold` | double | 0.0 | Adaptive: subdivide pixel when neighbor difference > threshold |

**Uniform** distributes `samples` evenly across each pixel.
**Adaptive** starts with a coarse sample and recursively subdivides regions where color
variation exceeds `threshold`.

---

## 9. Imports

A scene can include another `.cfg` file and optionally apply a transform to all its primitives:

```cfg
imports = (
    {
        path = "scenes/sub.cfg";

        # All transforms from section 4 are supported here too.
        # They are applied to every primitive imported from the file.
        scale = { x = 0.5; y = 0.5; z = 0.5; };
        translation = { x = 2.0; y = 0.0; z = 0.0; };
    }
);
```

- The imported file's camera is used only if the current file has none.
- Lights are always merged.
- Circular imports are detected and raise an error.
- Transforms on the import entry are applied in the standard order (Scale → Shear → Rotation → Translation → Matrix).

---

## 10. Value ranges & rules

| Field type | Valid range | Notes |
|------------|-------------|-------|
| Colors `r`, `g`, `b` | `[0.0, 1.0]` | Clamped at output |
| Radius `r` | `> 0` | - |
| Height `h` | `> 0` | - |
| Cone `angle` | `(0, π/2)` in radians | ≈ `(0, 1.5708)` |
| Rotation angles | radians | Applies to primitives; camera uses degrees |
| Scale components | `≠ 0` | Zero scale raises an error |
| Intensity | `> 0` | - |
| `samples` | `≥ 1` | 1 disables antialiasing |

---

## 11. Style guide

1. **Indentation**: 4 spaces
2. **Section delimiters**: prefer `=` for top-level keys, `:` for nested `children` blocks
3. **Semicolons**: required after every key-value pair and after closing `}`
4. **Commas**: between items in a list `( )`, no trailing comma after the last item
5. **Comments**: `#` at the start of the line; add a header block explaining the scene intent
6. **Numeric literals**: always include the decimal point for doubles (`1.0` not `1`)

---

## 12. Complete example

```cfg
# Annotated minimal scene with one group, a floor, and antialiasing.

camera = {
    resolution = { width = 800; height = 600; };
    position = { x = 0.0; y = 1.0; z = -6.0; };
    rotation = { x = 5.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};

renderer = {
    antialiasing = { samples = 4; type = "uniform"; };
};

primitives = {
    # Floor
    planes = (
        {
            x = 0.0; y = -1.0; z = 0.0;
            nx = 0.0; ny = 1.0; nz = 0.0;
            material = { type = "flat"; color = { r = 0.6; g = 0.6; b = 0.6; }; };
        }
    );

    # A cube in the center
    cubes = (
        {
            x = 0.0; y = 0.0; z = 3.0;
            s = 1.5;
            material = { type = "phong"; color = { r = 0.2; g = 0.6; b = 0.8; }; specular = { r = 1.0; g = 1.0; b = 1.0; }; shininess = 32.0; };
        }
    );

    # A group of two spheres moved together
    groups = (
        {
            translation = { x = 0.0; y = 0.5; z = 2.0; };
            children: {
                spheres = (
                    {
                        x = -1.2; y = 0.0; z = 0.0; r = 0.8;
                        material = { type = "flat"; color = { r = 0.9; g = 0.2; b = 0.2; }; };
                    },
                    {
                        x =  1.2; y = 0.0; z = 0.0; r = 0.8;
                        material = { type = "reflection"; color = { r = 0.8; g = 0.8; b = 1.0; }; };
                    }
                );
            };
        }
    );
};

lights = {
    ambient = (
        { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.15; }
    );
    directional = (
        { direction = { x = -1.0; y = -2.0; z = 1.0; }; color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.85; }
    );
};
```

For more examples see the [`scenes/`](../scenes/) directory.
