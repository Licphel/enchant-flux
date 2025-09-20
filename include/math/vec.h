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

} // namespace flux