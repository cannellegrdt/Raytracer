/*
 * Project: Raytracer
 * File name: Factory.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Generic factory mapping string keys to creator functions, with a PrimitiveFactory alias.
 */

#ifndef FACTORY_HPP_
    #define FACTORY_HPP_
    #include <memory>
    #include <functional>
    #include <unordered_map>
    #include <string>
    #include <optional>
    #include "Type.hpp"

/// @brief Generic factory for creating objects by string key.
/// @tparam T Base type to create.
/// @tparam Ptr Pointer type (default unique_ptr).
template<typename T, typename Ptr = std::unique_ptr<T>>
class Factory {
public:
    /// @brief Function pointer type for creating objects.
    using CreatorFunc = std::function<Ptr()>;

    /// @brief Registers a creator function for a key.
    /// @param key String identifier.
    /// @param creator Function that creates the object.
    void registerType(const std::string &key, CreatorFunc creator) {
        _creators[key] = std::move(creator);
    }

    /// @brief Creates an object by key.
    /// @param key String identifier.
    /// @return Optional containing created object, or empty if key not found.
    std::optional<Ptr> create(const std::string &key) const {
        auto it = _creators.find(key);
        if (it == _creators.end())
            return std::nullopt;
        return it->second();
    }

private:
    std::unordered_map<std::string, CreatorFunc> _creators;
};

/// @brief Factory specialization for primitives.
using PrimitiveFactory = Factory<IPrimitive, PrimitivePtr>;

#endif /* FACTORY_HPP_ */
