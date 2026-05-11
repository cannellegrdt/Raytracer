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

/// @brief RAII wrapper for dynamic library loading (dlopen).
/// @tparam T Type to create from the loaded library.
template<typename T>
class DLLoader {
public:
    /// @brief Function pointer type for object creation.
    /// @details Signature: T* create()
    using create_t = T* (*)();

    /// @brief Function pointer type for object destruction.
    /// @details Signature: void destroy(T*)
    using destroy_t = void (*)(T*);

    /// @brief Loads a shared library and resolves create/destroy symbols.
    /// @param libPath Path to .so file.
    /// @throws std::runtime_error if loading or symbol resolution fails.
    DLLoader(const std::string &libPath);

    DLLoader(const DLLoader&) = delete;
    DLLoader &operator=(const DLLoader&) = delete;
    DLLoader(DLLoader&&) = default;
    DLLoader &operator=(DLLoader&&) = default;

    /// @brief Gets the create function pointer.
    /// @return Function pointer to create objects.
    create_t  getCreate() const { return _create; }

    /// @brief Gets the destroy function pointer.
    /// @return Function pointer to destroy objects.
    destroy_t getDestroy() const { return _destroy; }

private:
    /// @brief RAII closer for dlclose handles.
    struct DlHandleCloser {
        /// @brief Closes a dynamic library handle.
        /// @param handle Handle to close.
        void operator()(void *handle) const noexcept {
            if (handle) dlclose(handle);
        }
    };

    /// @brief Unique pointer type for managing dlopen handles.
    using HandlePtr = std::unique_ptr<void, DlHandleCloser>;

    HandlePtr _handle;     ///< Dynamic library handle.
    create_t _create;      ///< Symbol for create function.
    destroy_t _destroy;    ///< Symbol for destroy function.
};

template<typename T>
DLLoader<T>::DLLoader(const std::string &libPath)
    : _handle(dlopen(libPath.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE))
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
