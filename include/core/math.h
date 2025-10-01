#pragma once
#include <cmath>

namespace flux
{

struct vec2
{
    double x = 0;
    double y = 0;

    vec2() = default;
    vec2(double x, double y) : x(x), y(y)
    {
    }

    vec2 operator+(const vec2 &v) const
    {
        return vec2(x + v.x, y + v.y);
    }
    vec2 operator-(const vec2 &v) const
    {
        return vec2(x - v.x, y - v.y);
    }
    vec2 operator*(const vec2 &v) const
    {
        return vec2(x * v.x, y * v.y);
    }
    vec2 operator/(const vec2 &v) const
    {
        return vec2(x / v.x, y / v.y);
    }
    vec2 operator+(double s) const
    {
        return vec2(x + s, y + s);
    }
    vec2 operator-(double s) const
    {
        return vec2(x - s, y - s);
    }
    vec2 operator*(double s) const
    {
        return vec2(x * s, y * s);
    }
    vec2 operator/(double s) const
    {
        return vec2(x / s, y / s);
    }

    double length() const
    {
        return sqrt(length_powered());
    }
    double length_powered() const
    {
        return x * x + y * y;
    }

    static vec2 from(double len, double rad)
    {
        double x = cos(rad) * len;
        double y = sin(rad) * len;
        return vec2(x, y);
    }

    vec2 normal() const
    {
        double len = length();
        return len > 0 ? *this / len : vec2();
    }

    static double dot(const vec2 &v1, const vec2 &v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }
};

struct vec3
{
    double x = 0;
    double y = 0;
    double z = 0;

    vec3() = default;
    vec3(double x, double y, double z) : x(x), y(y), z(z)
    {
    }

    vec3 operator+(const vec3 &v) const
    {
        return vec3(x + v.x, y + v.y, z + v.z);
    }
    vec3 operator-(const vec3 &v) const
    {
        return vec3(x - v.x, y - v.y, z - v.z);
    }
    vec3 operator*(const vec3 &v) const
    {
        return vec3(x * v.x, y * v.y, z * v.z);
    }
    vec3 operator/(const vec3 &v) const
    {
        return vec3(x / v.x, y / v.y, z / v.z);
    }
    vec3 operator+(double s) const
    {
        return vec3(x + s, y + s, z + s);
    }
    vec3 operator-(double s) const
    {
        return vec3(x - s, y - s, z - s);
    }
    vec3 operator*(double s) const
    {
        return vec3(x * s, y * s, z * s);
    }
    vec3 operator/(double s) const
    {
        return vec3(x / s, y / s, z / s);
    }

    double length() const
    {
        return sqrt(length_powered());
    }
    double length_powered() const
    {
        return x * x + y * y + z * z;
    }

    vec3 normal() const
    {
        double len = length();
        return len > 0 ? *this / len : vec3();
    }

    static double dot(const vec3 &v1, const vec3 &v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }
};

struct transform
{
    float m00 = 1.0;
    float m01 = 0.0;
    float m02 = 0.0;
    float m10 = 0.0;
    float m11 = 1.0;
    float m12 = 0.0;

    transform &identity()
    {
        m00 = 1.0;
        m01 = 0.0;
        m02 = 0.0;
        m10 = 0.0;
        m11 = 1.0;
        m12 = 0.0;
        return *this;
    }

    transform &load(const transform &o)
    {
        m00 = o.m00;
        m01 = o.m01;
        m02 = o.m02;
        m10 = o.m10;
        m11 = o.m11;
        m12 = o.m12;
        return *this;
    }

    transform &orthographic(float left, float right, float bottom, float top)
    {
        float sx = 2.0 / (right - left);
        float sy = 2.0 / (top - bottom);
        m00 = sx;
        m01 = 0.0;
        m02 = -(right + left) * sx * 0.5;
        m10 = 0.0;
        m11 = sy;
        m12 = -(top + bottom) * sy * 0.5;
        return *this;
    }

    transform &multiply(const transform &o)
    {
        float t00 = m00 * o.m00 + m01 * o.m10;
        float t01 = m00 * o.m01 + m01 * o.m11;
        float t02 = m00 * o.m02 + m01 * o.m12 + m02;
        float t10 = m10 * o.m00 + m11 * o.m10;
        float t11 = m10 * o.m01 + m11 * o.m11;
        float t12 = m10 * o.m02 + m11 * o.m12 + m12;
        m00 = t00;
        m01 = t01;
        m02 = t02;
        m10 = t10;
        m11 = t11;
        m12 = t12;
        return *this;
    }

    float det() const
    {
        return m00 * m11 - m01 * m10;
    }

    transform get_invert() const
    {
        float id = 1.0f / det();
        return transform{m11 * id,  -m01 * id, (m01 * m12 - m11 * m02) * id,
                         -m10 * id, m00 * id,  (m10 * m02 - m00 * m12) * id};
    }

    void apply(vec2 &v) const
    {
        double x = v.x, y = v.y;
        v.x = m00 * x + m01 * y + m02;
        v.y = m10 * x + m11 * y + m12;
    }

    transform &translate(float tx, float ty)
    {
        m02 += m00 * tx + m01 * ty;
        m12 += m10 * tx + m11 * ty;
        return *this;
    }

    transform &scale(float sx, float sy)
    {
        m00 *= sx;
        m01 *= sy;
        m10 *= sx;
        m11 *= sy;
        return *this;
    }

    transform &rotate(float rad)
    {
        float c = cos(rad), s = sin(rad);
        float r00 = m00 * c + m01 * s;
        float r01 = m00 * -s + m01 * c;
        float r10 = m10 * c + m11 * s;
        float r11 = m10 * -s + m11 * c;
        m00 = r00;
        m01 = r01;
        m10 = r10;
        m11 = r11;
        return *this;
    }

    transform &shear(float shx, float shy)
    {
        float n00 = m00 + shx * m01;
        float n01 = m01 + shy * m00;
        float n10 = m10 + shx * m11;
        float n11 = m11 + shy * m10;
        m00 = n00;
        m01 = n01;
        m10 = n10;
        m11 = n11;
        return *this;
    }
};

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

    quad &inflate_c(double nw, double nh)
    {
        width += nw;
        height += nh;
        x -= nw / 2.0;
        y -= nh / 2.0;
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