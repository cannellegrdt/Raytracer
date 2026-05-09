/*
 * Project: Raytracer
 * File name: test_obj.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Criterion unit tests for the ObjMesh plugin - file parsing, BVH construction, and ray intersection.
 */

#include <criterion/criterion.h>
#include <unordered_map>
#include <string>
#include <fstream>
#include <cstdlib>

#define create obj_create_fn
#define destroy obj_destroy_fn
#include "../../src/plugins/obj.cpp"
#undef create
#undef destroy

static std::shared_ptr<IMaterial> dummyMaterial() {
    struct DummyMat : public IMaterial {
        ScatterResult scatter(const Ray &, const HitRecord &) const override {
            return {{1.0, 1.0, 1.0}, std::nullopt, std::nullopt};
        }
    };
    return std::shared_ptr<IMaterial>(new DummyMat());
}

static std::string writeTempObj(const std::string &content) {
    static int counter = 0;
    std::string path = "/tmp/test_obj_" + std::to_string(++counter) + ".obj";
    std::ofstream f(path);
    f << content;
    f.close();
    return path;
}

static std::string simpleTriangleObj() {
    return R"(v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vn 0.0 0.0 1.0
f 1//1 2//1 3//1
)";
}

static std::string triangleNoNormalsObj() {
    return R"(v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2 3
)";
}

static std::string quadWithNegIdxObj() {
    return R"(v -1.0 -1.0 0.0
v  1.0 -1.0 0.0
v  1.0  1.0 0.0
v -1.0  1.0 0.0
f 1 2 3
f 1 3 4
)";
}

static std::string invalidObj() {
    return R"(this is not a valid OBJ file
just-些随机文本
)";
}

static std::string emptyObj() {
    return "";
}

static std::string commentsOnlyObj() {
    return R"(# This is a comment
# Another comment

)";
}

static std::string cubeLikeObj() {
    return R"(v -1.0 -1.0 -1.0
v  1.0 -1.0 -1.0
v  1.0  1.0 -1.0
v -1.0  1.0 -1.0
v -1.0 -1.0  1.0
v  1.0 -1.0  1.0
v  1.0  1.0  1.0
v -1.0  1.0  1.0
f 1 2 3
f 1 3 4
f 5 6 7
f 5 7 8
f 1 2 6
f 1 6 5
f 2 3 7
f 2 7 6
f 3 4 8
f 3 8 7
f 4 1 5
f 4 5 8
)";
}

Test(obj, plugin_create_returns_non_null) {
    IPrimitive *p = obj_create_fn();
    cr_assert_not_null(p);
    obj_destroy_fn(p);
}

Test(obj, plugin_destroy_deletes_object) {
    IPrimitive *p = obj_create_fn();
    cr_assert_no_throw(obj_destroy_fn(p));
}

Test(obj, configure_sets_filepath) {
    ObjMesh obj;
    std::unordered_map<std::string, double> params;
    obj.configure(params, dummyMaterial());
    cr_assert(true);
}

Test(obj, configure_with_material_stores_material) {
    ObjMesh obj;
    std::unordered_map<std::string, double> params;
    auto mat = dummyMaterial();
    obj.configure(params, mat);
    cr_assert(obj.intersect(Ray{{0,0,0}, {0,0,1}}).has_value() == false);
}

Test(obj, intersect_returns_nullopt_before_parse) {
    ObjMesh obj;
    obj.configure({}, dummyMaterial());
    Ray ray{{0, 0, 1}, {0, 0, -1}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, parse_valid_simple_triangle_no_throw) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
}

Test(obj, parse_valid_triangle_creates_aabb) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    AABB box = obj.boundingBox();
    cr_assert(box.min.x <= box.max.x);
}

Test(obj, parse_missing_file_throws) {
    ObjMesh obj;
    obj.setFilePath("/nonexistent/path/to/file.obj");
    cr_assert_throw(obj.configure({}, dummyMaterial()), std::runtime_error);
}

Test(obj, parse_invalid_content_returns_empty) {
    std::string path = writeTempObj(invalidObj());
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0,0,1}, {0,0,-1}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, parse_empty_file_returns_empty) {
    std::string path = writeTempObj(emptyObj());
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0,0,1}, {0,0,-1}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, parse_only_comments_returns_empty) {
    std::string path = writeTempObj(commentsOnlyObj());
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0,0,1}, {0,0,-1}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, ray_hits_simple_triangle) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
}

Test(obj, ray_misses_simple_triangle_outside) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{-1.0, -1.0, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, ray_parallel_to_triangle_no_hit) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {1.0, 0.0, 0.0}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, ray_pointing_away_no_hit) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, 1.0}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, hit_point_lies_at_triangle_surface) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->point.x, 0.25, 1e-10);
    cr_assert_float_eq(hit->point.y, 0.25, 1e-10);
    cr_assert_float_eq(hit->point.z, 0.0, 1e-10);
}

Test(obj, hit_point_matches_ray_at_t) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    Vec3 expected = ray.at(hit->t);
    cr_assert_float_eq(hit->point.x, expected.x, 1e-10);
    cr_assert_float_eq(hit->point.y, expected.y, 1e-10);
    cr_assert_float_eq(hit->point.z, expected.z, 1e-10);
}

Test(obj, normal_is_correct_direction) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->normal.x, 0.0, 1e-10);
    cr_assert_float_eq(hit->normal.y, 0.0, 1e-10);
    cr_assert_float_eq(hit->normal.z, 1.0, 1e-10);
}

Test(obj, normal_is_unit_length) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(length(hit->normal), 1.0, 1e-10);
}

Test(obj, front_face_true_on_front_hit) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->frontFace);
}

Test(obj, front_face_false_on_back_hit) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, -1.0}, {0.0, 0.0, 1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_not(hit->frontFace);
}

Test(obj, uv_coordinates_in_valid_range) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert(hit->UV.first >= 0.0 && hit->UV.first <= 1.0);
    cr_assert(hit->UV.second >= 0.0 && hit->UV.second <= 1.0);
}

Test(obj, uv_at_v0) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 0.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 0.0, 1e-10);
}

Test(obj, uv_at_v1) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{1.0, 0.0, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 1.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 0.0, 1e-10);
}

Test(obj, uv_at_v2) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.0, 1.0, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->UV.first, 0.0, 1e-10);
    cr_assert_float_eq(hit->UV.second, 1.0, 1e-10);
}

Test(obj, tangent_is_edge1_direction) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->tangent.x, 1.0, 1e-10);
    cr_assert_float_eq(hit->tangent.y, 0.0, 1e-10);
    cr_assert_float_eq(hit->tangent.z, 0.0, 1e-10);
}

Test(obj, degenerate_triangle_in_file_is_ignored) {
    std::string degenerate = R"(v 0.0 0.0 0.0
v 0.0 0.0 0.0
v 0.0 0.0 0.0
f 1 2 3
)";
    std::string path = writeTempObj(degenerate);
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0,0,1}, {0,0,-1}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, handles_negative_indices) {
    std::string path = writeTempObj(quadWithNegIdxObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
}

Test(obj, bvh_construction_allows_ray_hits) {
    std::string path = writeTempObj(cubeLikeObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.0, 0.0, 3.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
}

Test(obj, bvh_ignores_faces_behind_ray_origin) {
    std::string path = writeTempObj(simpleTriangleObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, -1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert_not(hit.has_value());
}

Test(obj, bvh_intersect_multiple_triangles_finds_nearest) {
    std::string two_tri = R"(v 0.0 0.0 -2.0
v 1.0 0.0 -2.0
v 0.0 1.0 -2.0
v 0.0 0.0 -4.0
v 1.0 0.0 -4.0
v 0.0 1.0 -4.0
f 1 2 3
f 4 5 6
)";
    std::string path = writeTempObj(two_tri);
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 0.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->t, 2.0, 1e-6);
}

Test(obj, interpolates_vertex_normals_correctly) {
    std::string variedNormals = R"(v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vn 1.0 0.0 0.0
vn 0.0 1.0 0.0
vn 0.0 0.0 1.0
f 1//1 2//2 3//3
)";
    std::string path = writeTempObj(variedNormals);
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
    cr_assert_float_eq(hit->normal.x, 0.816496580927726, 1e-4);
    cr_assert_float_eq(hit->normal.y, 0.408248290463863, 1e-4);
    cr_assert_float_eq(hit->normal.z, 0.408248290463863, 1e-4);
}

Test(obj, bounding_box_valid_after_parse) {
    std::string path = writeTempObj(cubeLikeObj());
    ObjMesh obj;
    obj.setFilePath(path);
    obj.configure({}, dummyMaterial());
    AABB box = obj.boundingBox();
    cr_assert(box.min.x < box.max.x);
    cr_assert(box.min.y < box.max.y);
    cr_assert(box.min.z < box.max.z);
}

Test(obj, multiple_faces_from_ngon) {
    std::string ngon = R"(v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 1.0 1.0 0.0
v 0.5 1.5 0.0
v -0.5 1.0 0.0
f 1 2 3 4 5
)";
    std::string path = writeTempObj(ngon);
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0.5, 0.5, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
}

Test(obj, ignores_polygon_faces_with_fewer_than_3_vertices) {
    std::string lineAndTri = R"(v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2
f 1 2 3
)";
    std::string path = writeTempObj(lineAndTri);
    ObjMesh obj;
    obj.setFilePath(path);
    cr_assert_no_throw(obj.configure({}, dummyMaterial()));
    Ray ray{{0.25, 0.25, 1.0}, {0.0, 0.0, -1.0}};
    auto hit = obj.intersect(ray);
    cr_assert(hit.has_value());
}
