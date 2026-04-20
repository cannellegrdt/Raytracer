/*
 * Project: Raytracer
 * File name: DLLoader.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: RAII template wrapper for dlopen/dlsym; loads a shared library and exposes typed create/destroy function pointers.
 */

#ifndef DLLOADER_HPP_
    #define DLLOADER_HPP_
    #include <memory>
    #include <string>
    #include <dlfcn.h>
    #include <stdexcept>

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
    create_t _create;
    destroy_t _destroy;
};

template<typename T>
DLLoader<T>::DLLoader(const std::string &libPath)
    : _handle(dlopen(libPath.c_str(), RTLD_NOW | RTLD_LOCAL))
{
    if (!_handle)
        throw std::runtime_error(dlerror());

    dlerror();
    _create = reinterpret_cast<create_t>(dlsym(_handle.get(), "create"));
    if (const char *err = dlerror())
        throw std::runtime_error(err);

    dlerror();
    _destroy = reinterpret_cast<destroy_t>(dlsym(_handle.get(), "destroy"));
    if (const char *err = dlerror())
        throw std::runtime_error(err);
}

#endif /* DLLOADER_HPP_ */
