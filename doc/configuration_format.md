# Configuration file format (.cfg)

This document describes the format and standards for scene configuration files in the Raytracer project.

## Overview

Configuration files use a custom syntax to define scenes with camera, primitives, and lighting. The format is parsed by `libconfigparser` and follows specific structural rules.

## File extension

- Use `.cfg` for all scene configuration files

## General syntax

- Sections are defined with `section_name = { ... }` or `section_name: { ... }`
- Key-value pairs use `key = value;` format
- Array/lists use `( ... )` with comma-separated items
- Vectors and colors use `{ x; y; z; }` and `{ r; g; b; }` syntax
- Comments start with `#` and are at the beginning of lines

## Sections

### Camera

```cfg
camera = {
    resolution = { width = 1920; height = 1080; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 72.0;
};
```

| Field | Type | Description |
|-------|------|------------|
| resolution | Vec2 | Output image dimensions |
| position | Vec3 | Camera position in world space |
| rotation | Vec3 | Camera rotation (pitch, yaw, roll) in degrees? |
| fieldOfView | double | Field of view in degrees |

### Primitives

Defines geometric objects in the scene. Each primitive type has its own section.

#### Sphere

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
|-------|------|------------|
| x, y, z | double | Center position |
| r | double | Radius (must be > 0) |
| material | Material | Surface material |

#### Plane

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
|-------|------|------------|
| x, y, z | double | Point on plane |
| nx, ny, nz | double | Surface normal (auto-normalized) |
| material | Material | Surface material |

#### Cylinder (infinite)

```cfg
cylinders = (
    {
        x = 4.0; y = 0.0; z = 6.0;
        ax = 0.0; ay = 1.0; az = 0.0;
        r = 0.5;
        material = { type = "flat"; color = { r = 0.2; g = 0.8; b = 0.2; }; };
    }
);
```

| Field | Type | Description |
|-------|------|------------|
| x, y, z | double | Center position |
| ax, ay, az | double | Axis direction (auto-normalized) |
| r | double | Radius (must be > 0) |
| material | Material | Surface material |

#### Limited cylinder

Same as cylinder with additional `h` parameter for height.

#### Cone (infinite)

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
|-------|------|------------|
| x, y, z | double | Apex position |
| ax, ay, az | double | Axis direction (auto-normalized) |
| angle | double | Half-angle in radians (must be in (0, pi/2)) |
| material | Material | Surface material |

#### Limited cone

Same as cone with additional `h` parameter for height.

### Lights

#### Ambient light

```cfg
ambient = (
    {
        color = { r = 1.0; g = 1.0; b = 1.0; };
        intensity = 0.15;
    }
);
```

| Field | Type | Description |
|-------|------|------------|
| color | Color | Light color (RGB in [0.0, 1.0]) |
| intensity | double | Light intensity (positive) |

#### Directional light

```cfg
directional = (
    {
        direction = { x = -0.5; y = -1.0; z = 0.5; };
        color = { r = 1.0; g = 0.95; b = 0.9; };
        intensity = 0.85;
    }
);
```

| Field | Type | Description |
|-------|------|------------|
| direction | Vec3 | Direction (auto-normalized) |
| color | Color | Light color (RGB in [0.0, 1.0]) |
| intensity | double | Light intensity (positive) |

### Material

Every primitive includes a material definition:

```cfg
material = { type = "flat"; color = { r = 1.0; g = 0.2; b = 0.2; }; };
```

| Field | Type | Description |
|-------|------|------------|
| type | string | Material type: "flat", "reflection", "transparency" |
| color | Color | Surface color (RGB in [0.0, 1.0]) |

## Value Ranges

| Type | Range | Notes |
|------|-------|-------|
| Colors (r, g, b) | [0.0, 1.0] | Vec3 / Color type |
| Angles | radians | Cone angle must be in (0, pi/2) |
| Intensity | positive double | Must be > 0 |
| Radius | positive double | Must be > 0 |
| Height | positive double | Must be > 0 |

## Standards

1. **Formatting**: Use 4 spaces for indentation
2. **Section style**: Use `=` for sections (both styles work, but be consistent)
3. **Trailing semicolons**: Always use semicolons after key-value pairs
4. **Commas**: Use commas between array items (except last)
5. **Curly braces**: Always use semicolons inside braces before closing
6. **Comments**: Use `#` at line start for comments
7. **Header comment**: Include a header with basic info and value ranges
8. **Section comments**: Add comments before major sections explaining parameters

## Example

If you want example, check [scenes/showcase.cfg](./../scenes/showcase.cfg).
