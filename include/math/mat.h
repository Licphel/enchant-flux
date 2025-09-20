#pragma once
#include <cmath>
#include <math/vec.h>

namespace flux
{

struct transform
{
    float m00 = 1.0;
    float m01 = 0.0;
    float m02 = 0.0;
    float m10 = 0.0;
    float m11 = 1.0;
    float m12 = 0.0;

    transform() = default;

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

    void apply(vec2& v) const
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

}