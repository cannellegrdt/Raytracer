/*
 * Project: Raytracer
 * File name: Factories.hpp
 * Author: Cannelle Gourdet - lankley
 * File description: Aggregates all concrete factory type aliases (LoaderFactory).
 */

#ifndef FACTORIES_HPP_
    #define FACTORIES_HPP_
    #include "Factory.hpp"
    #include "ISceneLoader.hpp"

using LoaderFactory = Factory<ISceneLoader>;

#endif /* FACTORIES_HPP_ */
