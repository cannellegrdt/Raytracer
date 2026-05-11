/*
 * Project: Raytracer
 * File name: Type.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Shared type aliases and helpers for primitive ownership (PrimitivePtr with custom deleter).
 */

#ifndef TYPE_HPP_
    #define TYPE_HPP_
    #include <memory>
    #include "IPrimitive.hpp"

/// @brief Unique pointer to IPrimitive with custom deleter.
/// @details Used for polymorphic primitive ownership with proper cleanup.
using PrimitivePtr = std::unique_ptr<IPrimitive, void(*)(IPrimitive*)>;

/// @brief Default deleter for primitive types.
/// @tparam T Concrete primitive type.
/// @param p Pointer to delete.
template<typename T>
void defaultDestroy(IPrimitive *p) { delete static_cast<T*>(p); }

#endif /* TYPE_HPP_ */
