/*
 * Project: Raytracer
 * File name: TransformBuilder.cpp
 * Author: Cannelle Gourdet - lankley
 * File description: Implementation of TransformBuilder for applying transformations to primitives.
 */

#include <libconfig.h++>
#include <memory>
#include "TransformBuilder.hpp"
#include "ConfigUtils.hpp"
#include "Decorators.hpp"
#include "Mat4.hpp"

PrimitivePtr TransformBuilder::applyTransforms(PrimitivePtr p, const libconfig::Setting &elem) {
    if (elem.exists("scale")) {
        const libconfig::Setting &s = elem["scale"];
        p = PrimitivePtr(
            new ScaleDecorator(std::move(p), Vec3{
                ConfigUtils::toDouble(s["x"]),
                ConfigUtils::toDouble(s["y"]),
                ConfigUtils::toDouble(s["z"])
            }),
            &defaultDestroy<ScaleDecorator>
        );
    }
    if (elem.exists("shear")) {
        const libconfig::Setting &sh = elem["shear"];
        p = PrimitivePtr(
            new ShearDecorator(std::move(p),
                ConfigUtils::toDouble(sh["sxy"]), ConfigUtils::toDouble(sh["sxz"]),
                ConfigUtils::toDouble(sh["syx"]), ConfigUtils::toDouble(sh["syz"]),
                ConfigUtils::toDouble(sh["szx"]), ConfigUtils::toDouble(sh["szy"])),
            &defaultDestroy<ShearDecorator>
        );
    }
    if (elem.exists("rotation")) {
        const libconfig::Setting &r = elem["rotation"];
        p = PrimitivePtr(
            new RotationDecorator(std::move(p), Vec3{
                ConfigUtils::toDouble(r["x"]),
                ConfigUtils::toDouble(r["y"]),
                ConfigUtils::toDouble(r["z"])
            }),
            &defaultDestroy<RotationDecorator>
        );
    }
    if (elem.exists("translation")) {
        const libconfig::Setting &t = elem["translation"];
        p = PrimitivePtr(
            new TranslationDecorator(std::move(p), Vec3{
                ConfigUtils::toDouble(t["x"]),
                ConfigUtils::toDouble(t["y"]),
                ConfigUtils::toDouble(t["z"])
            }),
            &defaultDestroy<TranslationDecorator>
        );
    }
    if (elem.exists("transformation_matrix")) {
        const libconfig::Setting &tm = elem["transformation_matrix"];
        Mat4 matrix{};
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                matrix.m[row][col] = ConfigUtils::toDouble(tm[row][col]);
        p = PrimitivePtr(
            new TransformMatrixDecorator(std::move(p), matrix),
            &defaultDestroy<TransformMatrixDecorator>
        );
    }
    return p;
}