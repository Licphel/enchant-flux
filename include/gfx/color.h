#pragma once
#include <algorithm>
#include <iostream>
#include <core/def.h>

namespace flux::gfx
{

struct color
{
    double r = 1.0;
    double g = 1.0;
    double b = 1.0;
    double a = 1.0;

    color() = default;

    color(double x, double y, double z, double w = 1.0) : r(x), g(y), b(z), a(w)
    {
    }

    static color from_bytes(byte r, byte g, byte b, byte a)
    {
        return color(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    }

    void get_bytes(byte *br, byte *bg, byte *bb, byte *ba)
    {
        *br = (byte)(255 * r);
        *bg = (byte)(255 * g);
        *bb = (byte)(255 * b);
        *ba = (byte)(255 * a);
    }

    color operator*(const color &v) const
    {
        return color(r * v.r, g * v.g, b * v.b, a * v.a);
    }
    // color operation, with a number, does not touch alpha.
    color operator+(double s) const
    {
        return color(r + s, g + s, b + s, a);
    }
    color operator-(double s) const
    {
        return color(r - s, g - s, b - s, a);
    }
    color operator*(double s) const
    {
        return color(r * s, g * s, b * s, a);
    }
    color operator/(double s) const
    {
        return color(r / s, g / s, b / s, a);
    }
};

} // namespace flux