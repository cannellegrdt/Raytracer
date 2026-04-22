#!/bin/bash
##
## Project: Raytracer
## File name: functional_tests.sh
## Author: Cannelle Gourdet - lankley
## File description: Functional tests for the raytracer binary.
##

BINARY="./raytracer"
PASS=0
FAIL=0

GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
BOLD="\033[1m"
RESET="\033[0m"

TMP_DIR=$(mktemp -d)
trap "rm -rf $TMP_DIR" EXIT

pass() { echo -e "  ${GREEN}[PASS]${RESET} $1"; PASS=$((PASS + 1)); }
fail() { echo -e "  ${RED}[FAIL]${RESET} $1"; FAIL=$((FAIL + 1)); }
skip() { echo -e "  ${YELLOW}[SKIP]${RESET} $1"; }
section() { echo -e "\n${BOLD}=== $1 ===${RESET}"; }

# ── Scene helpers (minimal 16×16 scenes for speed) ───────────────────────────

make_sphere_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres = ( { x = 0.0; y = 0.0; z = 0.0; r = 1.0; material = { type = "flat"; color = { r = 1.0; g = 0.25; b = 0.25; }; }; } );
    planes  = ();
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.4; } );
    directional = ();
    point = ();
};
EOF
}

make_plane_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = 5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres = ();
    planes  = ( { x = 0.0; y = 0.0; z = -1.0; nx = 0.0; ny = 0.0; nz = 1.0; material = { type = "flat"; color = { r = 0.25; g = 1.0; b = 0.25; }; }; } );
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.5; } );
    directional = ();
    point = ();
};
EOF
}

make_cylinder_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres   = ();
    planes    = ();
    cylinders = ( { x = 0.0; y = 0.0; z = 0.0; ax = 0.0; ay = 1.0; az = 0.0; r = 1.0; material = { type = "flat"; color = { r = 0.25; g = 0.25; b = 1.0; }; }; } );
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.4; } );
    directional = ();
    point = ();
};
EOF
}

make_cone_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres = ();
    planes  = ();
    cones   = ( { x = 0.0; y = 0.0; z = 0.0; ax = 0.0; ay = 1.0; az = 0.0; angle = 0.5236; material = { type = "flat"; color = { r = 1.0; g = 1.0; b = 0.25; }; }; } );
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.4; } );
    directional = ();
    point = ();
};
EOF
}

make_lights_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres = ( { x = 0.0; y = 0.0; z = 0.0; r = 1.0; material = { type = "flat"; color = { r = 0.78; g = 0.78; b = 0.78; }; }; } );
    planes  = ();
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.3; } );
    directional = ( { direction = { x = 1.0; y = -1.0; z = 0.5; }; color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 1.0; } );
    point       = ( { position = { x = 3.0; y = 3.0; z = -3.0; }; color = { r = 1.0; g = 0.78; b = 0.39; }; intensity = 1.0; } );
};
EOF
}

make_translate_scene() {
    cat > "$1" << 'EOF'
camera = {
    resolution = { width = 16; height = 16; };
    position = { x = 0.0; y = 0.0; z = -5.0; };
    rotation = { x = 0.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};
primitives = {
    spheres = (
        {
            x = 0.0; y = 0.0; z = 0.0; r = 0.8;
            material = { type = "flat"; color = { r = 1.0; g = 0.5; b = 0.0; }; };
            translation = { x = 1.0; y = 0.0; z = 0.0; };
        }
    );
    planes = ();
};
lights = {
    ambient = ( { color = { r = 1.0; g = 1.0; b = 1.0; }; intensity = 0.5; } );
    directional = ();
    point = ();
};
EOF
}

# ── Helpers ───────────────────────────────────────────────────────────────────

is_valid_ppm() {
    local file="$1"
    [ -f "$file" ] && [ -s "$file" ] || return 1
    local magic
    magic=$(head -c 2 "$file")
    [ "$magic" = "P3" ] || [ "$magic" = "P6" ]
}

ppm_dimensions() {
    awk '/^#/{next} NR==1{next} NR==2{print; exit}' "$1"
}

run_scene() {
    local cfg="$1" ppm="$2"
    "$BINARY" "$cfg" > "$ppm" 2>/dev/null
    echo $?
}

# ── 1. Binary ─────────────────────────────────────────────────────────────────
section "Binary"

if [ -x "$BINARY" ]; then
    pass "Binary '$BINARY' exists and is executable"
else
    fail "Binary '$BINARY' not found or not executable"
    echo -e "\n${RED}Cannot run tests without the binary. Aborting.${RESET}"
    exit 84
fi

# ── 2. Argument handling ──────────────────────────────────────────────────────
section "Argument handling"

# No arguments → exit 84
"$BINARY" >/dev/null 2>&1
[ $? -eq 84 ] && pass "No arguments → exit 84" || fail "No arguments → expected exit 84, got $?"

# No arguments → message on stderr
stderr=$("$BINARY" 2>&1 1>/dev/null)
[ -n "$stderr" ] && pass "No arguments → usage on stderr" || fail "No arguments → expected stderr message"

# --help → exit 0
"$BINARY" --help >/dev/null 2>&1
[ $? -eq 0 ] && pass "--help → exit 0" || fail "--help → expected exit 0, got $?"

# --help → produces output
output=$("$BINARY" --help 2>&1)
[ -n "$output" ] && pass "--help → produces output" || fail "--help → expected output"

# Non-existent file → exit 84
"$BINARY" /nonexistent/scene.cfg >/dev/null 2>&1
[ $? -eq 84 ] && pass "Non-existent file → exit 84" || fail "Non-existent file → expected exit 84, got $?"

# Non-existent file → error on stderr
stderr=$("$BINARY" /nonexistent/scene.cfg 2>&1 1>/dev/null)
[ -n "$stderr" ] && pass "Non-existent file → error on stderr" || fail "Non-existent file → expected stderr message"

# Unknown extension → exit 84
touch "$TMP_DIR/scene.xyz"
"$BINARY" "$TMP_DIR/scene.xyz" >/dev/null 2>&1
[ $? -eq 84 ] && pass "Unknown extension (.xyz) → exit 84" || fail "Unknown extension → expected exit 84, got $?"

# Empty .cfg → exit 84
touch "$TMP_DIR/empty.cfg"
"$BINARY" "$TMP_DIR/empty.cfg" >/dev/null 2>&1
[ $? -eq 84 ] && pass "Empty .cfg → exit 84" || fail "Empty .cfg → expected exit 84, got $?"

# Malformed .cfg → exit 84
echo "not valid libconfig { !!!" > "$TMP_DIR/bad.cfg"
"$BINARY" "$TMP_DIR/bad.cfg" >/dev/null 2>&1
[ $? -eq 84 ] && pass "Malformed .cfg → exit 84" || fail "Malformed .cfg → expected exit 84, got $?"

# Extra argument → no segfault
"$BINARY" /nonexistent.cfg extra_arg >/dev/null 2>&1
[ $? -ne 139 ] && pass "Extra argument → no segfault" || fail "Extra argument → segfault (SIGSEGV)"

# ── 3. PPM output — Sphere ────────────────────────────────────────────────────
section "PPM output — Sphere (MUST)"

SPHERE_CFG="$TMP_DIR/sphere.cfg"
SPHERE_PPM="$TMP_DIR/sphere.ppm"
make_sphere_scene "$SPHERE_CFG"
ec=$(run_scene "$SPHERE_CFG" "$SPHERE_PPM")

[ "$ec" -eq 0 ]            && pass "Sphere scene → exit 0"        || fail "Sphere scene → expected exit 0, got $ec"
is_valid_ppm "$SPHERE_PPM" && pass "Sphere scene → valid PPM header" || fail "Sphere scene → invalid or missing PPM"
[ -s "$SPHERE_PPM" ]       && pass "Sphere scene → non-empty output" || fail "Sphere scene → empty output"

dims=$(ppm_dimensions "$SPHERE_PPM")
echo "$dims" | grep -qE "^16[[:space:]]+16$" \
    && pass "Sphere scene → PPM dimensions match scene (16×16)" \
    || fail "Sphere scene → unexpected dimensions: '$dims'"

# ── 4. PPM output — Plane ─────────────────────────────────────────────────────
section "PPM output — Plane (MUST)"

PLANE_CFG="$TMP_DIR/plane.cfg"
PLANE_PPM="$TMP_DIR/plane.ppm"
make_plane_scene "$PLANE_CFG"
ec=$(run_scene "$PLANE_CFG" "$PLANE_PPM")

[ "$ec" -eq 0 ]            && pass "Plane scene → exit 0"        || fail "Plane scene → expected exit 0, got $ec"
is_valid_ppm "$PLANE_PPM" && pass "Plane scene → valid PPM header" || fail "Plane scene → invalid or missing PPM"

# ── 5. PPM output — Cylinder ──────────────────────────────────────────────────
section "PPM output — Cylinder (SHOULD)"

CYL_CFG="$TMP_DIR/cylinder.cfg"
CYL_PPM="$TMP_DIR/cylinder.ppm"
make_cylinder_scene "$CYL_CFG"
ec=$(run_scene "$CYL_CFG" "$CYL_PPM")

[ "$ec" -eq 0 ]           && pass "Cylinder scene → exit 0"         || fail "Cylinder scene → expected exit 0, got $ec"
is_valid_ppm "$CYL_PPM"  && pass "Cylinder scene → valid PPM header" || fail "Cylinder scene → invalid or missing PPM"

# ── 6. PPM output — Cone ─────────────────────────────────────────────────────
section "PPM output — Cone (SHOULD)"

CONE_CFG="$TMP_DIR/cone.cfg"
CONE_PPM="$TMP_DIR/cone.ppm"
make_cone_scene "$CONE_CFG"
ec=$(run_scene "$CONE_CFG" "$CONE_PPM")

[ "$ec" -eq 0 ]           && pass "Cone scene → exit 0"         || fail "Cone scene → expected exit 0, got $ec"
is_valid_ppm "$CONE_PPM" && pass "Cone scene → valid PPM header" || fail "Cone scene → invalid or missing PPM"

# ── 7. Lighting ───────────────────────────────────────────────────────────────
section "Lighting (MUST)"

LIT_CFG="$TMP_DIR/lights.cfg"
LIT_PPM="$TMP_DIR/lights.ppm"
make_lights_scene "$LIT_CFG"
ec=$(run_scene "$LIT_CFG" "$LIT_PPM")

[ "$ec" -eq 0 ]          && pass "Lights scene (ambient + directional + point) → exit 0" \
                          || fail "Lights scene → expected exit 0, got $ec"
is_valid_ppm "$LIT_PPM" && pass "Lights scene → valid PPM header" \
                          || fail "Lights scene → invalid or missing PPM"

# ── 8. Translation decorator ──────────────────────────────────────────────────
section "Translation decorator (MUST)"

TR_CFG="$TMP_DIR/translate.cfg"
TR_PPM="$TMP_DIR/translate.ppm"
make_translate_scene "$TR_CFG"
ec=$(run_scene "$TR_CFG" "$TR_PPM")

[ "$ec" -eq 0 ]         && pass "Translation scene → exit 0"        || fail "Translation scene → expected exit 0, got $ec"
is_valid_ppm "$TR_PPM" && pass "Translation scene → valid PPM header" || fail "Translation scene → invalid or missing PPM"

# ── 9. Output stream ──────────────────────────────────────────────────────────
section "Output stream"

SPHERE_CFG2="$TMP_DIR/sphere2.cfg"
make_sphere_scene "$SPHERE_CFG2"

stdout=$("$BINARY" "$SPHERE_CFG2" 2>/dev/null)
echo "$stdout" | head -c 2 | grep -qE "^P[36]" \
    && pass "Valid scene → PPM written to stdout" \
    || fail "Valid scene → PPM not found on stdout"

stderr=$("$BINARY" "$SPHERE_CFG2" 2>&1 1>/dev/null)
[ -z "$stderr" ] \
    && pass "Valid scene → no unexpected output on stderr" \
    || fail "Valid scene → unexpected stderr: $stderr"

# ── 10. Plugin system ─────────────────────────────────────────────────────────
section "Plugin system"

if ls plugins/*.so >/dev/null 2>&1; then
    pass "plugins/ directory contains .so files"
    PLG_PPM="$TMP_DIR/plugin_check.ppm"
    ec=$(run_scene "$SPHERE_CFG" "$PLG_PPM")
    [ "$ec" -eq 0 ] && is_valid_ppm "$PLG_PPM" \
        && pass "Render with plugins loaded → valid PPM" \
        || fail "Render with plugins loaded → failed (exit $ec)"
else
    skip "No .so files in plugins/ — plugin tests skipped"
fi

# ── 11. Demo scenes ───────────────────────────────────────────────────────────
section "Demo scenes (scenes/*.cfg)"

DEMO_FOUND=0
for scene in scenes/*.cfg; do
    [ -f "$scene" ] || continue
    DEMO_FOUND=1
    ppm_out="$TMP_DIR/demo_$(basename "${scene%.cfg}").ppm"
    "$BINARY" "$scene" > "$ppm_out" 2>/dev/null
    ec=$?
    name=$(basename "$scene")
    [ "$ec" -eq 0 ] && is_valid_ppm "$ppm_out" \
        && pass "$name → exit 0, valid PPM" \
        || fail "$name → exit $ec or invalid PPM"
done

[ $DEMO_FOUND -eq 0 ] && skip "No demo scenes found in scenes/ — add .cfg files to enable these tests"

# ── Summary ───────────────────────────────────────────────────────────────────
TOTAL=$((PASS + FAIL))
echo ""
echo "════════════════════════════════════════"
echo -e " Results: ${GREEN}${PASS} passed${RESET} / ${RED}${FAIL} failed${RESET} / ${TOTAL} total"
echo "════════════════════════════════════════"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
