#pragma once
#include <algorithm>

namespace flux
{

struct quad
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    quad() = default;

    quad(double x, double y, double w, double h) : x(x), y(y), width(w), height(h)
    {
    }

    static quad center(double x, double y, double w, double h)
    {
        return quad(x - w / 2.0, y - h / 2.0, w, h);
    }

    static quad corner(double x, double y, double w, double h)
    {
        return quad(x, y, w, h);
    }

    static quad conjunct(const quad &q1, const quad &q2)
    {
        double xc = std::max(q1.corner_x(), q2.corner_x());
        double yc = std::max(q1.corner_y(), q2.corner_y());
        double xc1 = std::max(q1.prom_x(), q2.prom_x());
        double yc1 = std::max(q1.prom_y(), q2.prom_y());
        return corner(xc, yc, xc1 - xc, yc1 - yc);
    }

    static bool intersect(const quad &q1, const quad &q2)
    {
        return conjunct(q1, q2).area() > 0;
    }

    quad &translate(double nx, double ny)
    {
        x += nx;
        y += ny;
        return *this;
    }

    quad &locate_center(double nx, double ny)
    {
        x = nx - width / 2.0;
        y = ny - height / 2.0;
        return *this;
    }

    quad &locate_corner(double nx, double ny)
    {
        x = nx;
        y = ny;
        return *this;
    }

    quad &resize(double nw, double nh)
    {
        width = nw;
        height = nh;
        return *this;
    }

    quad &expand(double nw, double nh)
    {
        width += nw;
        height += nh;
        return *this;
    }

    quad &scale(double nw, double nh)
    {
        width *= nw;
        height *= nh;
        return *this;
    }

    double center_x() const
    {
        return x + width / 2.0;
    }

    double center_y() const
    {
        return y + height / 2.0;
    }

    double corner_x() const
    {
        return x;
    }

    double corner_y() const
    {
        return y;
    }

    double prom_x() const
    {
        return x + width;
    }

    double prom_y() const
    {
        return y + height;
    }

    double area() const
    {
        return width * height;
    }
};

} // namespace flux