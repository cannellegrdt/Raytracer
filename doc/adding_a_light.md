# Adding a new light source

This guide walks through creating a new light type for the raytracer. Like materials, lights are **not** plugins : they are compiled directly into the core binary.

---

## Overview

1. Create a `.hpp` header and `.cpp` source in `include/lights/` and `src/lights/`
2. Implement the `ILight` interface
3. Register it in `LightsParser::parseAndAddLights()`
4. Rebuild with `make`

---

## Step-by-step: create a SpotLight

### 1. Create the header

Create `include/lights/SpotLight.hpp`:

```cpp
#ifndef SPOTLIGHT_HPP_
#define SPOTLIGHT_HPP_
#include "ILight.hpp"
#include "Color.hpp"

class SpotLight : public ILight {
public:
    SpotLight(const Vec3 &position, const Vec3 &direction, const Color &color,
              double intensity, double innerAngle, double outerAngle)
        : _position(position), _direction(normalize(direction)),
          _color(color), _intensity(intensity),
          _innerAngle(innerAngle), _outerAngle(outerAngle) {}

    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Vec3 _position;
    Vec3 _direction;
    Color _color;
    double _intensity;
    double _innerAngle;
    double _outerAngle;
};

#endif
```

### 2. Create the implementation

Create `src/lights/SpotLight.cpp`:

```cpp
#include "SpotLight.hpp"
#include "Common.hpp"
#include <cmath>
#include <limits>

LightSample SpotLight::getSample(const Vec3 &hitPoint, const Vec3 &/*normal*/) const {
    Vec3 toLight = _position - hitPoint;
    double dist = length(toLight);

    if (dist < 1e-8) {
        return LightSample{Vec3(0, 0, 0), Vec3(0, 0, 0), dist};
    }

    Vec3 dir = normalize(toLight);
    double cosTheta = dot(-dir, _direction);

    // Compute spotlight falloff (smooth edge between inner and outer cone)
    double innerCos = std::cos(_innerAngle);
    double outerCos = std::cos(_outerAngle);
    double falloff = (cosTheta - outerCos) / (innerCos - outerCos);
    falloff = std::clamp(falloff, 0.0, 1.0);

    if (falloff <= 0.0) {
        return LightSample{dir, Vec3(0, 0, 0), dist};
    }

    // Quadratic attenuation with spotlight falloff
    Vec3 attenuatedColor = _color * _intensity * falloff / (dist * dist);

    return LightSample{dir, attenuatedColor, dist};
}
```

### 3. Register in LightsParser

In `src/parser/LightsParser.cpp`, add the include:

```cpp
#include "SpotLight.hpp"
```

Then add a branch in `LightsParser::parseAndAddLights()`:

```cpp
} else if (name == "spot") {
    Vec3 position{
        ConfigUtils::toDouble(elem["position"]["x"]),
        ConfigUtils::toDouble(elem["position"]["y"]),
        ConfigUtils::toDouble(elem["position"]["z"])
    };
    Vec3 direction{
        ConfigUtils::toDouble(elem["direction"]["x"]),
        ConfigUtils::toDouble(elem["direction"]["y"]),
        ConfigUtils::toDouble(elem["direction"]["z"])
    };
    if (length(direction) < epsilon)
        throw std::invalid_argument("SpotLight direction cannot be null");
    double innerAngle = ConfigUtils::toDouble(elem["innerAngle"]);
    double outerAngle = ConfigUtils::toDouble(elem["outerAngle"]);
    scene.addLight(std::make_unique<SpotLight>(
        position, direction, color, intensity, innerAngle, outerAngle));
}
```

### 4. Rebuild

```sh
make re
```

### 5. Use in a scene file

```cfg
lights = {
    spot = (
        {
            position = { x = 0.0; y = 5.0; z = -3.0; };
            direction = { x = 0.0; y = -1.0; z = 0.5; };
            color = { r = 1.0; g = 1.0; b = 1.0; };
            intensity = 1.0;
            innerAngle = 0.3;
            outerAngle = 0.6;
        }
    );
};
```

---

## Required interface

Your class must inherit from `ILight` (`include/interfaces/ILight.hpp`):

```cpp
class MyLight : public ILight {
public:
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;
};
```

### `LightSample` fields

```cpp
struct LightSample {
    Vec3 direction;                                               // Direction from hit point toward the light
    Vec3 color;                                                   // Light color/intensity at the hit point
    double distance;                                              // Distance to light source (infinity = no attenuation)
    bool isAmbient = false;                                       // True = no shadow ray needed
    double maxDistance = std::numeric_limits<double>::infinity(); // Max distance for AO rays
};
```

**Key fields explained:**

- **`direction`**: Unit vector from the hit point toward the light. Used for diffuse (Lambert) and specular (Phong) calculations, and for casting shadow rays.
- **`color`**: The light's color after attenuation. For point/spot lights, divide by distance². For directional/ambient lights, return the raw color × intensity.
- **`distance`**: Distance from hit point to the light. Use `std::numeric_limits<double>::infinity()` for directional/ambient lights (no distance attenuation). The renderer uses this for shadow ray max distance.
- **`isAmbient`**: Set to `true` for ambient-like lights that should not cast shadow rays.
- **`maxDistance`**: For ambient lights, the maximum distance for ambient occlusion sampling rays.

---

## Common patterns

### Directional light (no attenuation)

```cpp
LightSample getSample(const Vec3 &/*hitPoint*/, const Vec3 &/*normal*/) const {
    return LightSample{
        -_direction,                              // Light comes FROM this direction
        _color * _intensity,                      // No distance attenuation
        std::numeric_limits<double>::infinity()   // Infinite distance
    };
}
```

### Point light (quadratic attenuation)

```cpp
LightSample getSample(const Vec3 &hitPoint, const Vec3 &/*normal*/) const {
    Vec3 dir = _position - hitPoint;
    double dist = length(dir);
    return LightSample{
        normalize(dir),
        _color * _intensity / (dist * dist),      // Inverse-square falloff
        dist
    };
}
```

### Ambient light (no direction, no shadows)

```cpp
LightSample getSample(const Vec3 &/*hitPoint*/, const Vec3 &/*normal*/) const {
    return LightSample{
        Vec3(0, 0, 0),                            // No direction
        _color * _intensity,
        std::numeric_limits<double>::infinity(),
        true,                                      // isAmbient = true (skip shadow rays)
        _maxDist                                   // AO sampling distance
    };
}
```

---

## Parameters from the `.cfg` file

`LightsParser` iterates over the `lights` group. Each child's `name()` is the light type key (e.g. `"ambient"`, `"directional"`, `"point"`). You read fields from the `libconfig::Setting` with `ConfigUtils::toDouble()`:

```cpp
Color color{
    ConfigUtils::toDouble(elem["color"]["r"]),
    ConfigUtils::toDouble(elem["color"]["g"]),
    ConfigUtils::toDouble(elem["color"]["b"])
};
double intensity = ConfigUtils::toDouble(elem["intensity"]);
```

Use `elem.exists("fieldName")` for optional fields.

---

## Tips

- The `getSample()` method is called for every hit point × every light, so keep it fast
- Always normalize direction vectors before returning them in `LightSample`
- Use `std::numeric_limits<double>::infinity()` for distance when there is no attenuation
- If your light has no meaningful direction (ambient), set `isAmbient = true` to skip shadow ray tests
- Test with a simple scene: one sphere, one of your new lights, and verify the output visually
- Run `make unit_tests` to verify nothing is broken
