/*
 * Project: Raytracer
 * File name: IPrimitive.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Interface for geometric primitives; defines ray-intersection returning an optional HitRecord.
 */

#ifndef IPRIMITIVE_HPP_
    #define IPRIMITIVE_HPP_
    #include <optional>
    #include <stdexcept>
    #include <unordered_map>
    #include <string>
    #include "HitRecord.hpp"
    #include "Ray.hpp"

class IMaterial;

class IPrimitive {
public:
    virtual std::optional<HitRecord> intersect(const Ray &ray) const = 0;
    virtual void configure(const std::unordered_map<std::string, double> &params, const IMaterial *mat) = 0;
    virtual ~IPrimitive() = default;
};

#endif /* IPRIMITIVE_HPP_ */
