```cpp
// ============================================================
//  BASIC TYPES
// ============================================================

struct Vec3 {
    double x, y, z;
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

class IMaterial;

struct HitRecord {
    double t;
    Vec3 point;
    Vec3 normal;
    const IMaterial *material;
    bool frontFace;
};

struct ScatterResult {
    Vec3 attenuation;
    std::optional<Ray> scatteredRay;
};

struct LightSample {
    Vec3 direction;
    Vec3 color;
    double distance;
};


// ============================================================
//  INTERFACES
// ============================================================

class IPrimitive {
public:
    virtual std::optional<HitRecord> intersect(const Ray &ray) const = 0;
    virtual ~IPrimitive() = default;
};

class IMaterial {
public:
    virtual ScatterResult scatter(const Ray &ray, const HitRecord &hit) const = 0;
    virtual ~IMaterial() = default;
};

class ILight {
public:
    virtual LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const = 0;
    virtual ~ILight() = default;
};


// ============================================================
//  PRIMITIVE PTR
//  The deleter is embedded in the pointer:
//  - delete for built-ins
//  - destroy() from the .so file for plugins
// ============================================================

using PrimitivePtr = std::unique_ptr<IPrimitive, std::function<void(IPrimitive*)>>;

template<typename T>
static void defaultDestroy(IPrimitive *p) { delete static_cast<T*>(p); }


// ============================================================
//  GENERIC FACTORY
// ============================================================

template<typename T, typename Ptr = std::unique_ptr<T>>
class Factory {
public:
    using CreatorFunc = std::function<Ptr()>;

    void registerType(const std::string &key, CreatorFunc creator) {
        creators_[key] = std::move(creator);
    }

    std::optional<Ptr> create(const std::string &key) {
        auto it = creators_.find(key);
        if (it == creators_.end())
            return std::nullopt;
        return it->second();
    }

private:
    std::unordered_map<std::string, CreatorFunc> creators_;
};

using PrimitiveFactory = Factory<IPrimitive, PrimitivePtr>;
using LoaderFactory = Factory<ISceneLoader>;


// ============================================================
//  LOADING PLUGIN
//
//  Required entry points in each .so file:
//    extern "C" IPrimitive* create();
//    extern "C" void destroy(IPrimitive* ptr);
// ============================================================

template<typename T>
class DLLoader {
public:
    using create_t = T* (*)();
    using destroy_t = void (*)(T*);

    DLLoader(const std::string &libPath);

    DLLoader(const DLLoader&) = delete;
    DLLoader &operator=(const DLLoader&) = delete;
    DLLoader(DLLoader&&) = default;
    DLLoader &operator=(DLLoader&&) = default;

    create_t  getCreate() const { return _create; }
    destroy_t getDestroy() const { return _destroy; }

private:
    struct DlHandleCloser {
        void operator()(void *handle) const noexcept {
            if (handle) dlclose(handle);
        }
    };
    using HandlePtr = std::unique_ptr<void, DlHandleCloser>;

    HandlePtr _handle;
    create_t  _create;
    destroy_t _destroy;
};

class PluginLoader {
public:
    void loadAll(const std::string &pluginsDir, PrimitiveFactory &factory);

private:
    std::vector<DLLoader<IPrimitive>> loadedPlugins_;
};


// ============================================================
//  SCENE LOADER
// ============================================================

class SceneContext;

class ISceneLoader {
public:
    virtual SceneContext load(const std::string &filePath, PrimitiveFactory &factory) = 0;
    virtual ~ISceneLoader() = default;
};

class LibconfigLoader : public ISceneLoader {
public:
    SceneContext load(const std::string &filePath, PrimitiveFactory &factory) override;
};


// ============================================================
//  SCENE
// ============================================================

class Scene {
public:
    void addPrimitive(PrimitivePtr primitive);
    void addLight(std::unique_ptr<ILight> light);

    const std::vector<PrimitivePtr> &primitives() const { return primitives_; }
    const std::vector<std::unique_ptr<ILight>> &lights() const { return lights_; }

private:
    std::vector<PrimitivePtr> primitives_;
    std::vector<std::unique_ptr<ILight>> lights_;
};


// ============================================================
//  CAMERA
// ============================================================

class Camera {
public:
    Camera(Vec3 position, Vec3 rotation, double fov, int width, int height);
    Ray generateRay(int x, int y) const;
};

struct SceneContext {
    Scene scene;
    Camera camera;
};


// ============================================================
//  PRIMITIVE BUILDER
//  Creates a primitive and chains decorators
// ============================================================

class PrimitiveBuilder {
public:
    explicit PrimitiveBuilder(Factory<IPrimitive> &factory) : _factory(factory) {}

    PrimitiveBuilder &setType(const std::string &type);
    PrimitiveBuilder &setMaterial(std::shared_ptr<IMaterial> material);

    PrimitiveBuilder &setTranslation(const Vec3 &t);
    PrimitiveBuilder &setRotation(const Vec3 &r);
    PrimitiveBuilder &setScale(const Vec3 &s);

    PrimitivePtr build();
    /*
     * build() :
     *   1. factory.create(_type) → Basic PrimitivePtr
     *   2. if (_translation) wrap in TranslationDecorator
     *   3. if (_rotation) wrap in RotationDecorator
     *   4. if (_scale) wrap in ScaleDecorator
     *   5. return final PrimitivePtr
     */

private:
    PrimitiveFactory &_factory;
    std::string _type;
    std::shared_ptr<IMaterial> _material;
    std::optional<Vec3> _translation;
    std::optional<Vec3> _rotation;
    std::optional<Vec3> _scale;
};


// ============================================================
//  COLOR UTILITY
// ============================================================

struct Color {
    double r, g, b;
};

inline Color clamp(const Color &c) {
    return { std::clamp(c.r, 0.0, 1.0),
             std::clamp(c.g, 0.0, 1.0),
             std::clamp(c.b, 0.0, 1.0) };
}

inline void writePPM(std::ostream &out, const Color &c) {
    out << static_cast<int>(c.r * 255) << ' '
        << static_cast<int>(c.g * 255) << ' '
        << static_cast<int>(c.b * 255) << '\n';
}


// ============================================================
//  CONCRETE MATERIALS
// ============================================================

class FlatColor : public IMaterial {
public:
    explicit FlatColor(const Color &color) : _color(color) {}
    ScatterResult scatter(const Ray &ray, const HitRecord &hit) const override;

private:
    Color _color;
};


// ============================================================
//  CONCRETE LIGHTS
// ============================================================

class AmbientLight : public ILight {
public:
    AmbientLight(const Color &color, double intensity);
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Color _color;
    double _intensity;
};

class DirectionalLight : public ILight {
public:
    DirectionalLight(const Vec3 &direction, const Color &color, double intensity);
    LightSample getSample(const Vec3 &hitPoint, const Vec3 &normal) const override;

private:
    Vec3 _direction;
    Color _color;
    double _intensity;
};


// ============================================================
//  DECORATORS (Design Pattern: Decorator)
//  Each wraps a PrimitivePtr and transforms the ray into
//  object space before delegating to the inner intersect().
// ============================================================

class TranslationDecorator : public IPrimitive {
public:
    TranslationDecorator(PrimitivePtr inner, const Vec3 &offset);
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _offset;
};

class RotationDecorator : public IPrimitive {
public:
    RotationDecorator(PrimitivePtr inner, const Vec3 &angles);
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _angles;
};

class ScaleDecorator : public IPrimitive {
public:
    ScaleDecorator(PrimitivePtr inner, const Vec3 &scale);
    std::optional<HitRecord> intersect(const Ray &ray) const override;

private:
    PrimitivePtr _inner;
    Vec3 _scale;
};


// ============================================================
//  RENDERER
// ============================================================

class Renderer {
public:
    void render(const SceneContext &context);

private:
    Color traceRay(const Ray &ray, const Scene &scene, int depth) const;
};


// ============================================================
//  MAIN - guaranteed standard of living
// ============================================================
//
//  int main(int argc, char **argv) {
//      // 1. Factories
//      PrimitiveFactory primitiveFactory;
//      LoaderFactory loaderFactory;
//
//      // 2. Built-ins
//      primitiveFactory.registerType("sphere", []() {
//          return PrimitivePtr(new Sphere(), &defaultDestroy<Sphere>);
//      });
//      primitiveFactory.registerType("plane", []() {
//          return PrimitivePtr(new Plane(), &defaultDestroy<Plane>);
//      });
//
//      // 3. Scene loaders
//      loaderFactory.registerType(".cfg", []() {
//          return std::make_unique<LibconfigLoader>();
//      });
//
//      // 4. Plugins - loaded AFTER the built-ins
//      PluginLoader pluginLoader;
//      pluginLoader.loadAll("./plugins", primitiveFactory);
//
//      // 5. Parse
//      std::string path = argv[1];
//      std::string ext = path.substr(path.rfind('.'));
//      auto loader = loaderFactory.create(ext);
//      SceneContext ctx = loader->load(path, primitiveFactory);
//
//      // 6. Render
//      Renderer renderer;
//      renderer.render(ctx);
//
//      return 0;
//  }
```
