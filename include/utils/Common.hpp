/*
 * Project: Raytracer
 * File name: Common.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Shared constants and utility definitions used across the project.
 */

#ifndef COMMON_HPP_
    #define COMMON_HPP_

    #include <cmath>

    /// @brief Global epsilon for floating-point comparisons.
    constexpr double epsilon = 1e-12;

    /// @brief Ray bias offset to avoid self-intersection (shadow acne).
    constexpr double RayBias = 1e-4;

#endif /* COMMON_HPP_ */
