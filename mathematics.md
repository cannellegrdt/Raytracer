# Raytracer - Mathematics reference

This document presents the mathematical foundations of every component: how rays are cast, how each primitive computes intersections and normals, how lights contribute to the final color, and how materials scatter or transmit light.

---

## Table of contents

1. [The ray tracing algorithm](#1-the-ray-tracing-algorithm)
2. [Ray generation](#2-ray-generation)
3. [Primitives](#3-primitives)
   - [Sphere](#31-sphere)
   - [Plane](#32-plane)
   - [Cylinder](#33-cylinder)
   - [Cone](#34-cone)
   - [Torus](#35-torus)
   - [Triangle](#36-triangle)
   - [Tanglecube](#37-tanglecube)
   - [Mandelbulb](#38-mandelbulb)
   - [Möbius strip](#39-möbius-strip)
4. [Geometric transformations (Decorator pattern)](#4-geometric-transformations-decorator-pattern)
   - [General principle](#41-general-principle)
   - [Translation](#42-translation)
   - [Rotation](#43-rotation)
   - [Scale](#44-scale)
   - [Shear](#45-shear)
   - [Arbitrary 4×4 matrix](#46-arbitrary-44-matrix)
   - [Application order](#47-application-order)
5. [Lights and the lighting equation](#5-lights-and-the-lighting-equation)
   - [Ambient light and ambient occlusion](#51-ambient-light-and-ambient-occlusion)
   - [Directional light](#52-directional-light)
   - [Point light](#53-point-light)
6. [Materials](#6-materials)
   - [Flat color](#61-flat-color)
   - [Phong material](#62-phong-material)
   - [Reflection](#63-reflection)
   - [Refraction](#64-refraction)
   - [Transparency](#65-transparency)
   - [Textured material](#66-textured-material)
   - [Procedural checkerboard](#67-procedural-checkerboard)
   - [Procedural marble](#68-procedural-marble)
   - [Normal map material](#69-normal-map-material)
7. [Acceleration: BVH](#7-acceleration-bvh)

---

## 1. The ray tracing algorithm

A ray tracer simulates the path of light in reverse: instead of following photons from light sources to the camera, it traces rays from the camera into the scene.

### The rendering equation (simplified)

$$L(p,\ \omega_o) = L_e(p,\ \omega_o) + \int f_r(p,\ \omega_i,\ \omega_o) * L_i(p,\ \omega_i) (\omega_i * n) * d\omega_i$$

Where:
- $L(p,\ \omega_o)$ - outgoing radiance at point $p$ in direction $\omega_o$
- $L_e$ - emitted light (light sources)
- $f_r$ - BRDF: how much light from $\omega_i$ is reflected toward $\omega_o$
- $L_i(p,\ \omega_i)$ - incoming radiance from direction $\omega_i$
- $(\omega_i * n)$ - Lambert cosine term

This raytracer approximates this integral with:
- **Direct lighting**: one sample per light (shadow ray)
- **Indirect lighting**: one recursive bounce (reflection/refraction/transparency)
- **Ambient occlusion**: Monte Carlo sampling over the hemisphere

### `traceRay` pseudocode

```
traceRay(ray, scene, depth):
    hit = closestHit(ray, scene)           // BVH or brute-force
    if no hit: return backgroundColor

    scatter = hit.material.scatter(ray, hit)
    effectiveNormal = scatter.modifiedNormal ?? hit.normal

    color = (0, 0, 0)
    for each light:
        sample = light.getSample(hit.point, hit.normal)
        if not blocked by shadow ray:
            color += Lambert(effectiveNormal, sample) * scatter.attenuation
            color += Phong(scatter, sample, ray)

    if depth > 0 and scatter.secondaryRay:
        indirect = traceRay(scatter.secondaryRay, scene, depth - 1)
        color += scatter.attenuation * indirect

    return color
```

**Key constants:**

| Constant | Value | Purpose |
|----------|-------|---------|
| `MAX_DEPTH` | 10 | Maximum recursion depth |
| `RayBias` | 1×10⁻⁴ | Offset origin to avoid self-intersection |
| `epsilon` | 1×10⁻¹² | Minimum valid `t` value |

---

## 2. Ray generation

A ray is defined by an origin $O$ and a normalized direction $d$: $P(t) = O + t*d,$ with $t > 0$

`Camera::generateRay(x, y)` maps pixel coordinates to world-space rays using:
1. The camera's field of view (vertical FOV in degrees)
2. The camera's position and Euler rotation matrix
3. The aspect ratio `width / height`

The pixel `(x, y)` is mapped to normalized device coordinates, then to the camera's local view frustum, and finally rotated into world space by the camera's rotation matrix.

For antialiasing, sub-pixel coordinates are used:
* $u = x + \frac{(i + 0.5)}{N}$ with $i \in [0, N)$
* $v = y + \frac{(j + 0.5)}{N}$ with $j \in [0, N)$


producing $N^2$ rays per pixel, whose colors are averaged.

---

## 3. Primitives

All intersection tests find the smallest positive $t$ such that $P(t) = O + t*d$ lies on the surface. The surface normal at that point is used for shading.

---

### 3.1 Sphere

**Definition:** center $C$, radius $r$.

A point $P$ is on the sphere when $|P - C|^2 = r^2$.

**Intersection:**

Substitute $P = O + t*d$:
$\ \ \ \ |O + t*d - C|^2 = r^2$

Let $OC = O - C$:
$\ \ \ \ |d|^2*t^2 + 2(d * OC)t + |OC|^2 - r^2 = 0$

Using the half-discriminant form (numerically stable):
$\ \ \ \ a = d * d$
$\ \ \ \ h = d * OC$
$\ \ \ \ c = OC * OC - r^2$

$\Delta = h^2 - a*c$

- If $\Delta < 0$: no intersection
- Otherwise:
    - $t_1 = \frac{(-h - \sqrt{\Delta})}{a}$
    - $t_2 = \frac{(-h + \sqrt{\Delta})}{a}$
    Take the smallest positive root.

**Normal** at hit point $P$: `n = normalize(P - C)`

**UV mapping** (spherical projection):
- $u = 0.5 + \frac{atan(nz,\ nx)}{2\pi}$
- $v = 0.5 + \frac{asin(ny)}{\pi}$

where $(nx,\ ny,\ nz)$ is the unit normal at the hit point.

---

### 3.2 Plane

**Definition:** point $C$ on the plane, unit normal $n$.

A point $P$ is on the plane when $n * (P - C) = 0$.

**Intersection:**
$n * (O + t*d - C) = 0$
$t = n * \frac{(C - O)}{(n * d)}$

- If $n * d \approx 0$: ray is parallel to the plane → no intersection
- Otherwise: test $t > epsilon$

**Normal:** always $\pm n$ (sign depends on `frontFace`, which indicates if the ray hits the front or back)

**UV mapping:**

A tangent frame $(T, B)$ is built from $n$ (the vector perpendicular to $n$ with the smallest component is used to bootstrap the frame via Gram-Schmidt). UV coordinates are the projected distances along $T$ and $B$.

---

### 3.3 Cylinder

**Definition:** axis direction $a$ (unit vector), center point $C$, radius $r$.

The cylinder is the locus of points at distance $r$ from the axis.

**Intersection:**

Decompose $d$ and $OC = O - C$ perpendicular to the axis:
$\ \ \ \ d' = d - (d * a) a$
$\ \ \ \ OC' = OC - (OC * a) a$

This reduces to a 2D circle intersection in the plane perpendicular to the axis:
$\ \ \ \ |d'|^2 * t^2 + 2(d' * OC') t + |OC'|^2 - r^2 = 0$

Solve with the standard quadratic formula. Check caps (if the cylinder is bounded) by intersecting the ray with the two cap planes $C \pm halfHeight * a$.

**Normal:**

For the lateral surface, the normal is the radial component of $(P - C)$:
`n = normalize(P - C - ((P-C)*a)*a)`

For caps: $n = \pm\ a$

---

### 3.4 Cone

**Definition:** apex $C$, axis direction $a$, half-angle $\theta$.

A point $P$ is on the cone when the angle between $(P-C)$ and $a$ equals $\theta$:
$\ \ \ \ (P-C) * a = |P - C| * cos(\theta)$

Squaring:
$\ \ \ \ ((P-C)*a)^2 = cos^2(\theta) * |P-C|^2$

Let $k = cos^2(\theta)$, and $OC = O - C$:
$\ \ \ \ a_{coef} = (d * a)^2 - k$
$\ \ \ \ b_{coef} = (d * a)(OC * a) - k(d * OC)$
$\ \ \ \ c_{coef} = (OC * a)^2 - k|OC|^2$

$\Delta = b_{coef}^2 - a_{coef} * c_{coef}$

Take the smallest valid root. For bounded cones, test against the cap plane.

**Normal:**

Let $h = (P-C)*a$ (projection along axis) and `radial = normalize(P-C)`: `n = normalize(h*a - k*radial)`

This is the gradient of the implicit cone equation.

---

### 3.5 Torus

**Definition:** major radius $R$ (center of tube to center of torus), minor radius $r$ (tube radius).

**Implicit equation** (in the local frame where the torus lies in the XZ plane):
$\ \ \ \ (x^2 + y^2 + z^2 + R^2 - r^2)^2 = 4R^2(x^2 + z^2)$

**Intersection:**

Substituting $P(t) = O + t*d$ into the implicit equation yields a **degree-4 polynomial** in $t$:
$\ \ \ \ A*t^4 + B*t^3 + C*t^2 + D*t + E = 0$
where the coefficients depend on:
$\ \ \ \ |d|^2$, $d*O$, $|O|^2$, $R$, $r$.

This quartic is solved analytically via a **depressed cubic** (Ferrari's method):
1. Depress the quartic by substituting $t = s - \frac{B}{4A}$
2. Solve the resulting resolvent cubic
3. Recover the four roots of the quartic

Among the real roots, take the smallest positive one.

**Normal:**

The gradient of the implicit function $F(p) = (|p|^2 + R^2 - r^2)^2 - 4R^2(px^2 + pz^2)$:
`n = normalize(4p * (|p|² + R² - r²) - 8R²(px, 0, pz))`

---

### 3.6 Triangle

**Definition:** vertices $V_0$, $V_1$, $V_2$.

**Möller-Trumbore algorithm:**
$\ \ \ \ e_1 = V_1 - V_0$
$\ \ \ \ e_2 = V_2 - V_0$
$\ \ \ \ p = d * e_2$
$\ \ \ \ det = e_1 * p$

- If $|det| < epsilon$: ray is parallel → no intersection
$\ \ \ \ T = O - V_0$
$\ \ \ \ u = \frac{(T * p)}{det}$
$\ \ \ \ q = T * e_1$
$\ \ \ \ v = \frac{(d * q)}{det}$
$\ \ \ \ t = \frac{(e_2 * p)}{det}$

Valid intersection when:
$\ \ \ \ u \geq 0$
$\ \ \ \ v \geq 0$
$\ \ \ \ u+v \leq 1$
$\ \ \ \ t > epsilon$

**Normal:** `n = normalize(e₁ × e₂)`

If vertex normals are available (OBJ meshes), they are interpolated using barycentric coordinates: `n = normalize(u*n₁ + v*n₂ + (1-u-v)*n₀)`

---

### 3.7 Tanglecube

**Definition:** the algebraic surface defined by:
$\ \ \ \ F(p) = x^4 - 5x^2 + y^4 - 5y^2 + z^4 - 5z^2 + 11.8 = 0$
There is no closed-form ray intersection for this implicit surface.

**Sphere bounding test:** a fast bounding sphere is tested first; if the ray misses it, the intersection is skipped.

**Ray marching:** the ray is stepped in increments. The sign of $F(P(t))$ is tracked:

```
for step in 0..255:
    t += stepSize
    if F(P(t)) changes sign:
        // surface crossed - refine
        break
```

**Bisection refinement:** once a sign change is detected between $t_a$ and $t_b$, 64 bisection steps narrow the root:

```
for i in 0..63:
    t_mid = (t_a + t_b) / 2
    if sign(F(P(t_mid))) == sign(F(P(t_a))):
        t_a = t_mid
    else:
        t_b = t_mid
t ≈ t_mid
```

**Normal:**

The gradient of `F`:
```
∇F(p) = (4x³ - 10x,  4y³ - 10y,  4z³ - 10z)
n = normalize(∇F(p))
```

---

### 3.8 Mandelbulb

**Definition:** the 3D generalization of the Mandelbrot set. The iteration is:
$\ \ \ \ Z_0 = c = p$ (point being tested)
$\ \ \ \ z_{n+1} = z_n^{power} + c$

where the power operation uses **spherical coordinates**:
$\ \ \ \ r = |z|$
$\ \ \ \ \theta = acos(z.y / r)$
$\ \ \ \ \varphi = atan(z.z,\ z.x)$

$z^n$:
$\ \ \ \ r' = r^{power}$
$\ \ \ \ \theta' = power * \theta$
$\ \ \ \ \varphi' = power * \varphi$

$z^n = r' * (sin(\theta') * cos(\varphi'),\ cos(\theta'),\ sin(\theta') * sin(\varphi'))$

**Distance estimator (DE):**

While iterating, the derivative magnitude $dr$ is tracked:
$\ \ \ \ dr = power * r^{power-1} * dr + 1$

After `maxIter` iterations or when $r > bailout$:
$\ \ \ \ DE(p) \approx 0.5 * log(r) * r / dr$

This estimates the distance from point $p$ to the Mandelbulb surface.

**Ray marching:**

```
t = 0
for step in 0..999:
    p = O + t*d
    dist = DE(p)
    if dist < 1e-5: hit at t
    t += dist
    if t > maxDist: miss
```

**Normal** (finite differences, step $\varepsilon = 0.001$):

```
n = normalize(
    DE(p + (ε,0,0)) - DE(p - (ε,0,0)),
    DE(p + (0,ε,0)) - DE(p - (0,ε,0)),
    DE(p + (0,0,ε)) - DE(p - (0,0,ε))
)
```

---

### 3.9 Möbius Strip

**Definition:** major radius $R$, half-width $w$. Parametric equations (parameters $u \in [0, 2 \pi)$,$v \in [-w, w)$):
$\ \ \ \ x(u,\ v) = (R + v*cos(\frac{u}{2})) * cos(u)$
$\ \ \ \ y(u,\ v) = (R + v*cos(\frac{u}{2})) * sin(u)$
$\ \ \ \ z(u,\ v) = v * sin(\frac{u}{2})$

**Intersection:**

The ray $P(t) = O + t*d$ must satisfy $P(t) = M(u,\ v)$: a system of 3 equations in 3 unknowns $(t,\ u,\ v)$.

This nonlinear system is solved by **Newton-Raphson iteration**, repeated from 8 different starting points $u_0 \in \{0,\ \frac{\pi}{4},\ \frac{\pi}{2},\ ...,\ 7\frac{\pi}{4}\}$ to capture all branches:

```
for each starting (u₀, v₀, t₀):
    for iter in 0..14:
        F = M(u, v) - P(t)      // residual (3D vector)
        J = ∂F/∂(t, u, v)       // 3×3 Jacobian
        [Δt, Δu, Δv] = J⁻¹ * (-F)
        (t, u, v) += (Δt, Δu, Δv)
    if converged and t > epsilon and |v| ≤ w:
        record hit
```

Among all valid hits, the smallest positive $t$ is returned.

**Normal:**

The surface normal is the cross product of the two partial derivatives: $\frac{\partial M}{\partial u}$ and $\frac{\partial M}{\partial v}$
`n = normalize(∂M/∂u × ∂M/∂v)`

---

## 4. Geometric transformations (Decorator pattern)

Each transformation is implemented as a **decorator** that wraps any `IPrimitive`. Decorators are composable and stackable — wrapping a primitive with multiple decorators applies the transformations in sequence.

---

### 4.1 General principle

Rather than modifying the geometry of a primitive directly, the **inverse transformation is applied to the incoming ray**, the intersection is computed in **object space**, and the resulting hit point and normal are mapped back to **world space** with the forward transformation.

```
intersect(ray):
    localRay = T⁻¹(ray)           // bring ray into object space
    hit      = inner.intersect(localRay)
    hit.point  = T(hit.point)     // map point back to world space
    hit.normal = normalTransform(hit.normal)
    return hit
```

**Normal transformation — the inverse-transpose rule:**

If a point transforms under matrix $M$ (i.e. $p' = Mp$), then, to preserve orthogonality between tangent vectors and the surface normal after the transform, the normal must be transformed by the **inverse-transpose** of $M$:

$$n' = \text{normalize}\left((M^{-1})^T\, n\right)$$

This rule is applied consistently across all non-trivial decorators (scale, shear, arbitrary matrix).

---

### 4.2 Translation

**Parameters:** offset vector $t = (t_x, t_y, t_z)$

Translation shifts a point: $P' = P + t$. Directions are **not** affected by translation.

**Ray in object space:**
$$O' = O - t, \quad d' = d$$

**Inverse mapping back to world space:**
$$P_{world} = P'_{local} + t$$

**Normal:** unchanged — translation does not alter orientation.

---

### 4.3 Rotation

**Parameters:** Euler angles $(\alpha, \beta, \gamma)$ in radians for the X, Y, and Z axes.

The combined rotation matrix is built as $R = R_x(\alpha) \cdot R_y(\beta) \cdot R_z(\gamma)$:

$$R_x(\alpha) = \begin{pmatrix} 1 & 0 & 0 \\ 0 & \cos\alpha & -\sin\alpha \\ 0 & \sin\alpha & \cos\alpha \end{pmatrix}, \quad R_y(\beta) = \begin{pmatrix} \cos\beta & 0 & \sin\beta \\ 0 & 1 & 0 \\ -\sin\beta & 0 & \cos\beta \end{pmatrix}, \quad R_z(\gamma) = \begin{pmatrix} \cos\gamma & -\sin\gamma & 0 \\ \sin\gamma & \cos\gamma & 0 \\ 0 & 0 & 1 \end{pmatrix}$$

$R$ is **orthogonal**: $R^{-1} = R^T$.

**Ray in object space:**
$$O' = R^T O, \quad d' = R^T d$$

**Inverse mapping back to world space:**
$$P_{world} = R\, P'_{local}, \quad n_{world} = \text{normalize}(R\, n'_{local})$$

The normal uses the same $R$ (not $R^T$) because $(R^{-1})^T = (R^T)^T = R$.

---

### 4.4 Scale

**Parameters:** per-axis scale factors $(s_x, s_y, s_z)$, all non-zero.

The scale matrix is diagonal:

$$S = \begin{pmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & s_z \end{pmatrix}$$

**Ray in object space** (component-wise division by $S$):
$$O' = \left(\frac{O_x}{s_x},\, \frac{O_y}{s_y},\, \frac{O_z}{s_z}\right), \quad d' = \left(\frac{d_x}{s_x},\, \frac{d_y}{s_y},\, \frac{d_z}{s_z}\right)$$

**Inverse mapping back to world space:**
$$P_{world} = (P'_x \cdot s_x,\ P'_y \cdot s_y,\ P'_z \cdot s_z)$$

**Normal** — applying the inverse-transpose rule (for a diagonal matrix, $(S^{-1})^T = S^{-1}$):
$$n_{world} = \text{normalize}\left(\frac{n'_x}{s_x},\, \frac{n'_y}{s_y},\, \frac{n'_z}{s_z}\right)$$

The tangent and bitangent vectors are transformed identically to the normal.

---

### 4.5 Shear

**Parameters:** six shear factors $s_{xy},\, s_{xz},\, s_{yx},\, s_{yz},\, s_{zx},\, s_{zy}$.

Each factor $s_{ij}$ displaces coordinate $i$ in proportion to coordinate $j$. The shear matrix is:

$$S_h = \begin{pmatrix} 1 & s_{xy} & s_{xz} \\ s_{yx} & 1 & s_{yz} \\ s_{zx} & s_{zy} & 1 \end{pmatrix}$$

Its inverse $S_h^{-1}$ and the transposed inverse $(S_h^{-1})^T$ are precomputed at construction time.

**Ray in object space:**
$$O' = S_h^{-1}\, O, \quad d' = S_h^{-1}\, d$$

**Inverse mapping back to world space:**
$$P_{world} = S_h\, P'_{local}$$

**Normal** — inverse-transpose rule:
$$n_{world} = \text{normalize}\left((S_h^{-1})^T\, n'_{local}\right)$$

---

### 4.6 Arbitrary 4×4 matrix

**Parameters:** a user-supplied $4 \times 4$ homogeneous matrix $M$.

This decorator is the most general: any affine transform (or combination of them) can be expressed as a single $4 \times 4$ matrix. The inverse $M^{-1}$ is computed at construction time via Gaussian elimination with partial pivoting.

In homogeneous coordinates, **points** carry $w = 1$ (translation is applied), while **directions** carry $w = 0$ (translation is ignored):

$$\text{transformPoint}(M, p) = \begin{pmatrix} M_{00} p_x + M_{01} p_y + M_{02} p_z + M_{03} \\ M_{10} p_x + M_{11} p_y + M_{12} p_z + M_{13} \\ M_{20} p_x + M_{21} p_y + M_{22} p_z + M_{23} \end{pmatrix}$$

$$\text{transformDirection}(M, d) = \begin{pmatrix} M_{00} d_x + M_{01} d_y + M_{02} d_z \\ M_{10} d_x + M_{11} d_y + M_{12} d_z \\ M_{20} d_x + M_{21} d_y + M_{22} d_z \end{pmatrix}$$

**Ray in object space:**
$$O' = \text{transformPoint}(M^{-1}, O), \quad d' = \text{transformDirection}(M^{-1}, d)$$

**Inverse mapping back to world space:**
$$P_{world} = \text{transformPoint}(M, P'_{local})$$

**Normal** — inverse-transpose rule, using the upper-left $3 \times 3$ block of $M^{-1}$:
$$n_{world} = \text{normalize}\left((M^{-1}_{3 \times 3})^T\, n'_{local}\right)$$

---

### 4.7 Application order

`TransformBuilder::applyTransforms` wraps the primitive in the following order (innermost first):

| Step | Decorator applied | Object-to-world direction |
|------|-------------------|---------------------------|
| 1 | `ScaleDecorator` | scale first |
| 2 | `ShearDecorator` | then shear |
| 3 | `RotationDecorator` | then rotate |
| 4 | `TranslationDecorator` | then translate |
| 5 | `TransformMatrixDecorator` | then arbitrary matrix |

Because each decorator's `intersect` method calls the inner primitive, the **world-to-object** ray path is the reverse: the outermost decorator is entered first, so the ray is transformed by $M^{-1}$, then $T^{-1}$, then $R^T$, then $S_h^{-1}$, then $S^{-1}$ before reaching the primitive.

Not all decorators need to be present; only the transforms specified in the scene configuration file are applied.

---

## 5. Lights and the lighting equation

The total color at a surface point combines contributions from all lights:
$\ \ \ \ C_{final} = attenuation * (\sum_{lights} *\ C_{{diffuse}_{i}} + C_{ao}) + \sum_{lights} * C_{{specular}_{i}} + attenuation * C_{indirect}$

Each light provides a `LightSample` containing:
- `direction` - unit vector from hit point toward light
- `color` - light color $\times$ intensity
- `distance` - distance to light source ($\infty$ for directional)
- `isAmbient` - whether this is an AO light

---

### 5.1 Ambient light and ambient occlusion

Classical ambient light is a constant term that approximates indirect illumination globally. This raytracer replaces it with **ambient occlusion (AO)**: a local estimate of how much of the hemisphere above the surface point is unobstructed.

**Algorithm:**

```
unoccluded = 0
for i in 0..nbAORays:
    AODir = randomDirection in upper hemisphere of hit.normal
    AORay = Ray(hit.point + RayBias*n, AODir)
    if no intersection within maxDistance:
        unoccluded++

AOFactor = unoccluded / nbAORays
C_ambient = light.color * AOFactor
```

- `AOFactor = 1.0`: surface fully open → full ambient contribution
- `AOFactor = 0.0`: surface fully enclosed → no ambient

**Random hemisphere sampling:** a uniform random direction in the upper hemisphere is generated by sampling a point inside a unit sphere and flipping it if it points away from the normal:

```
p = random point in unit sphere
if p * n < 0: p = -p
AODir = normalize(p)
```

---

### 5.2 Directional light

A directional light simulates an infinitely distant source (e.g. the sun). All rays are parallel; there is **no distance attenuation**.

```
LightSample:
    direction = -lightDirection      // unit vector toward light
    color = lightColor * intensity
    distance = ∞
```

**Lambert diffuse contribution:**
$\ \ \ \ C_{diffuse} = (n * L) * lightColor$

where $L$ = `sample.direction`, $n$ = effective surface normal.

---

### 5.3 Point light

A point light radiates in all directions from a fixed position. Intensity falls off with the **inverse square of the distance**:

```
LightSample:
    direction = normalize(pos - hit.point)
    distance = |pos - hit.point|
    color = lightColor * intensity / distance²
```

**Lambert diffuse:**
$\ \ \ \ C_{diffuse} = (n * L) * \frac{lightColor}{distance^2}$

**Shadow ray:** cast from `hit.point + RayBias*n` in direction $L$. If any opaque object is closer than `distance`, the light is blocked. Transmissive objects are traversed with a `shadowFilter` color accumulator (up to 8 bounces):

```
shadowFilter = {1, 1, 1}
for each transmissive blocker:
    shadowFilter *= blocker.attenuation
```

---

## 6. Materials

A material defines what happens when a ray hits a surface: it returns an `attenuation` color and optionally a secondary ray for recursion.

The full shading at depth $d$:
$\ \ \ \ C = attenuation * (C_{direct}) + attenuation * traceRay(secondaryRay,\ d-1)$

---

### 6.1 Flat color

The simplest material: every ray returns the same color regardless of angle.

```
scatter:
    attenuation = color
    secondaryRay = none
```

No lighting model applied beyond the direct lighting accumulation in `traceRay`.

---

### 6.2 Phong material

Combines **Lambert diffuse** and **phong specular** shading.

**Lambert diffuse** (computed in `traceRay` for all materials):
$\ \ \ \ C_{diffuse} += (n * L) * lightColor * shadowFilter$

**Phong specular** (only for materials returning `getSpecular()`):

The specular highlight is centered on the **reflection of the light direction**:

```
R = reflect(-L, n) = -L + 2(L*n)n
viewDir = normalize(-rayDirection)

specAngle = max(0, R * viewDir)
C_specular += ks * lightColor * specAngle^shininess * shadowFilter
```

Where:
- `ks` - specular color coefficient
- `shininess` - controls highlight sharpness (higher = narrower)

---

### 6.3 Reflection

Perfect mirror reflection.

**Reflection formula:**

For incident direction $d$ and surface normal $n$:
$\ \ \ \ R = d - 2(d * n)n$
`R = normalize(R)`

```
scatter:
    attenuation = reflectionColor
    secondaryRay = Ray(hit.point + RayBias*n, R)
```

The recursive call to `traceRay` with `secondaryRay` produces the reflection image.

---

### 6.4 Refraction

Dielectric material (glass, water, crystal) implementing Snell-Descartes law with Schlick's Fresnel approximation.

**Snell-Descartes law:**
$\ \ \ \ \eta_1 * sin(\theta_1) = \eta_2 * sin(\theta_2)$
where $\theta_1$ is the angle of incidence and $\theta_2$ the angle of refraction.

**Implementation:**

```
etaRatio = frontFace ? (1.0 / ior) : ior      // air→glass or glass→air

cosθ = -d * n      // d is normalized, n points outward

// Perpendicular and parallel components of refracted ray
rPerp = etaRatio * (d + cosθ*n)
rParallel = -√(1 - |rPerp|²) * n
refracted = rPerp + rParallel
```

**Total internal reflection:** when $|rPerp|^2 > 1$, refraction is impossible and the ray reflects instead.

**Schlick's Fresnel approximation:**

At grazing angles, even glass reflects. Schlick approximates the reflectance:
$\ \ \ \ r_0 = \frac{1 - ior}{1 + ior}^2$
$\ \ \ \ reflectance = r_0 + (1 - r_0)(1 - cos(\theta))^5$

If `reflectance > random()` → reflect instead of refract.

```
scatter:
    attenuation = refractionColor
    secondaryRay = Ray(hit.point - RayBias*n, refracted_or_reflected)
```

---

### 6.5 Transparency

Simple transparency: the ray continues in the **same direction** through the surface.

Unlike refraction, there is no bending. The color is applied only on the front face:

```
scatter:
    attenuation = frontFace ? color : {1,1,1}
    secondaryRay = Ray(hit.point - RayBias*n, d)    // same direction, biased inward
```

This allows colored glass without index of refraction effects. In shadow ray traversal, transmissive objects accumulate their attenuation into `shadowFilter`.

---

### 6.6 Textured material

Applies an image texture using UV coordinates from the `HitRecord`.

UV coordinates $(u, v) \in [0,1]^2$ are mapped to pixel coordinates:

```
px = (int)(u * textureWidth) mod textureWidth
py = (int)(v * textureHeight) mod textureHeight
```

The texel color becomes the `attenuation`. No secondary ray is produced.

---

### 6.7 Procedural checkerboard

A 3D procedural pattern based on the **XOR parity** of the integer coordinates:

```
ix = (int)floor(p.x)
iy = (int)floor(p.y)
iz = (int)floor(p.z)

parity = (ix XOR iy XOR iz) & 1
color = parity ? colorA : colorB
```

where `p` is the world-space hit point scaled by the checker frequency. The XOR of three integers is odd when an odd number of them are odd, producing a 3D checkerboard in all planes simultaneously.

---

### 6.8 Procedural marble

**Fractal brownian motion (fBm):**

fBm sums multiple layers (octaves) of Perlin noise at increasing frequencies and decreasing amplitudes:

```
fBm(p, octaves):
    value = 0
    amplitude = 1
    frequency = 1
    for i in 0..octaves:
        value += amplitude * perlinNoise(p * frequency)
        amplitude *= 0.5
        frequency *= 2
    return value
```

**Marble pattern:**

The marble color is driven by a **sine wave** whose phase is perturbed by fBm:

```
noiseVal = sin(scale * p.z + turbulence * fBm(p, octaves))
t = (noiseVal + 1) / 2 // remap to [0, 1]
color = lerp(colorA, colorB, t)
```

- `scale` - frequency of the marble veins
- `turbulence` - amplitude of fBm perturbation
- `octaves` - detail levels

The sine creates bands; fBm makes them irregular, simulating the chaotic vein structure of natural marble.

---

### 6.9 Normal map material

Perturbs the surface normal using an RGB image encoding local normal directions.

**Tangent-bitangent-normal (TBN) frame:**

The TBN matrix transforms normals from tangent space (texture space) to world space:

```
TBN = [ T | B | N ]       // 3×3 matrix, column vectors
```

where:
- **T** = tangent vector (aligned with U texture axis)
- **B** = bitangent = N × T (aligned with V texture axis)
- **N** = geometric surface normal from `HitRecord`

These are precomputed in `HitRecord` by the primitive's intersection code.

**Normal perturbation:**

```
// Read normal map texel
texelColor = texture.sample(hit.uv)

// Decode: RGB [0,255] → XYZ [-1, +1]
tangentNormal = texelColor * 2 - 1

// Transform to world space
worldNormal = normalize(TBN * tangentNormal)
```

The world-space perturbed normal replaces `hit.normal` in all lighting calculations via `ScatterResult.modifiedNormal`.

---

## 7. Acceleration: BVH

**Bounding volume hierarchy (BVH)** reduces the ray–scene intersection cost from $O(n)$ to $O(log\ n)$.

**Construction (SAH - surface area heuristic):**

Each primitive is bounded by an **axis-aligned bounding box (AABB)**: `AABB(pMin, pMax)`

The BVH is built recursively:

```
build(primitives):
    if |primitives| ≤ threshold:
        return Leaf(primitives)

    // Find best split using SAH:
    // cost(split) = 1 + (SA(left)/SA(parent)) * N_left
    //                 + (SA(right)/SA(parent)) * N_right
    // where SA = surface area of AABB

    bestAxis, bestSplit = argmin_over_axes_and_positions(cost)
    [left, right] = partition(primitives, bestAxis, bestSplit)
    return Node(AABB(left ∪ right), build(left), build(right))
```

**Traversal:**

```
intersect(ray, node):
    if not ray.hits(node.aabb): return none
    if node is Leaf: return closestHit(ray, node.primitives)

    hitLeft = intersect(ray, node.left)
    hitRight = intersect(ray, node.right)
    return closer(hitLeft, hitRight)
```

**Unbounded primitives** (planes, infinite cylinders) return `AABB::infinite()` and are always tested directly, bypassing the BVH.

**AABB ray test:**

For each axis, compute the interval $[t_{min}, t_{max}]$ where the ray is inside the slab:

```
for axis in {x, y, z}:
    t0 = (pMin[axis] - O[axis]) / d[axis]
    t1 = (pMax[axis] - O[axis]) / d[axis]
    if d[axis] < 0: swap(t0, t1)
    tEnter = max(tEnter, t0)
    tExit  = min(tExit,  t1)

hit = tEnter ≤ tExit and tExit > 0
```

The ray enters the AABB at `tEnter` and exits at `tExit`. An intersection exists when these intervals overlap.
