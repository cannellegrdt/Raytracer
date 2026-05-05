# Configuration file format (.cfg)

This document describes the full syntax of scene configuration files used by the raytracer.
Files are parsed by **libconfig++** and loaded by `LibconfigLoader`.

---

## Table of contents

1. [File extension & structure](#1-file-extension--structure)
2. [Camera](#2-camera)
3. [Primitives](#3-primitives)
   - [Sphere](#sphere)
   - [Plane](#plane)
   - [Cylinder (infinite)](#cylinder-infinite)
   - [Limited cylinder](#limited-cylinder)
   - [Cone (infinite)](#cone-infinite)
   - [Limited cone](#limited-cone)
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
    ...
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

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | One of `"flat"`, `"reflection"`, `"transparency"` |
| `color` | Color | RGB in `[0.0, 1.0]` |

---

## 7. Lights

```cfg
lights = {
    ambient = (
        { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.2; }
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

### Directional

| Field | Type | Description |
|-------|------|-------------|
| `direction` | Vec3 | Direction of incoming light (auto-normalized) |
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
