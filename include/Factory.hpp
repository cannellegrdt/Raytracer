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

template<typename T, typename Ptr = std::unique_ptr<T>>
class Factory {
public:
    using CreatorFunc = std::function<Ptr()>;

    void registerType(const std::string &key, CreatorFunc creator) {
        creators_[key] = std::move(creator);
    }

    std::optional<Ptr> create(const std::string &key) const {
        auto it = creators_.find(key);
        if (it == creators_.end())
            return std::nullopt;
        return it->second();
    }

private:
    std::unordered_map<std::string, CreatorFunc> creators_;
};

using PrimitiveFactory = Factory<IPrimitive, PrimitivePtr>;

#endif /* FACTORY_HPP_ */
