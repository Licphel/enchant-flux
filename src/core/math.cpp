#include <core/math.h>

namespace flux
{

vec2::vec2() = default;

vec2::vec2(double x, double y) : x(x), y(y)
{
}

vec2 vec2::operator+(const vec2 &v) const
{
    return vec2(x + v.x, y + v.y);
}

vec2 vec2::operator-(const vec2 &v) const
{
    return vec2(x - v.x, y - v.y);
}

vec2 vec2::operator*(const vec2 &v) const
{
    return vec2(x * v.x, y * v.y);
}

vec2 vec2::operator/(const vec2 &v) const
{
    return vec2(x / v.x, y / v.y);
}

vec2 vec2::operator+(double s) const
{
    return vec2(x + s, y + s);
}

vec2 vec2::operator-(double s) const
{
    return vec2(x - s, y - s);
}

vec2 vec2::operator*(double s) const
{
    return vec2(x * s, y * s);
}

vec2 vec2::operator/(double s) const
{
    return vec2(x / s, y / s);
}

double vec2::length() const
{
    return std::sqrt(length_powered());
}

double vec2::length_powered() const
{
    return x * x + y * y;
}

vec2 vec2::from(double len, double rad)
{
    return vec2(std::cos(rad) * len, std::sin(rad) * len);
}

vec2 vec2::normal() const
{
    double len = length();
    return len > 0.0 ? *this / len : vec2();
}

double vec2::dot(const vec2 &v1, const vec2 &v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

vec3::vec3() = default;

vec3::vec3(double x, double y, double z) : x(x), y(y), z(z)
{
}

vec3 vec3::operator+(const vec3 &v) const
{
    return vec3(x + v.x, y + v.y, z + v.z);
}

vec3 vec3::operator-(const vec3 &v) const
{
    return vec3(x - v.x, y - v.y, z - v.z);
}

vec3 vec3::operator*(const vec3 &v) const
{
    return vec3(x * v.x, y * v.y, z * v.z);
}

vec3 vec3::operator/(const vec3 &v) const
{
    return vec3(x / v.x, y / v.y, z / v.z);
}

vec3 vec3::operator+(double s) const
{
    return vec3(x + s, y + s, z + s);
}

vec3 vec3::operator-(double s) const
{
    return vec3(x - s, y - s, z - s);
}

vec3 vec3::operator*(double s) const
{
    return vec3(x * s, y * s, z * s);
}

vec3 vec3::operator/(double s) const
{
    return vec3(x / s, y / s, z / s);
}

double vec3::length() const
{
    return std::sqrt(length_powered());
}

double vec3::length_powered() const
{
    return x * x + y * y + z * z;
}

vec3 vec3::normal() const
{
    double len = length();
    return len > 0.0 ? *this / len : vec3();
}

double vec3::dot(const vec3 &v1, const vec3 &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

transform::transform() = default;

transform::transform(float m00, float m01, float m02, float m10, float m11, float m12)
    : m00(m00), m01(m01), m02(m02), m10(m10), m11(m11), m12(m12)
{
}

transform &transform::identity()
{
    m00 = 1.0f;
    m01 = 0.0f;
    m02 = 0.0f;
    m10 = 0.0f;
    m11 = 1.0f;
    m12 = 0.0f;
    return *this;
}

transform &transform::load(const transform &o)
{
    *this = o;
    return *this;
}

transform &transform::orthographic(float left, float right, float bottom, float top)
{
    float sx = 2.0f / (right - left);
    float sy = 2.0f / (top - bottom);
    m00 = sx;
    m01 = 0.0f;
    m02 = -(right + left) * sx * 0.5f;
    m10 = 0.0f;
    m11 = sy;
    m12 = -(top + bottom) * sy * 0.5f;
    return *this;
}

transform &transform::multiply(const transform &o)
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

float transform::det() const
{
    return m00 * m11 - m01 * m10;
}

transform transform::get_invert() const
{
    float id = 1.0f / det();
    return transform(m11 * id, -m01 * id, (m01 * m12 - m11 * m02) * id, -m10 * id, m00 * id,
                     (m10 * m02 - m00 * m12) * id);
}

void transform::apply(vec2 &v) const
{
    double xx = v.x, yy = v.y;
    v.x = m00 * xx + m01 * yy + m02;
    v.y = m10 * xx + m11 * yy + m12;
}

transform &transform::translate(float tx, float ty)
{
    m02 += m00 * tx + m01 * ty;
    m12 += m10 * tx + m11 * ty;
    return *this;
}

transform &transform::scale(float sx, float sy)
{
    m00 *= sx;
    m01 *= sy;
    m10 *= sx;
    m11 *= sy;
    return *this;
}

transform &transform::rotate(float rad)
{
    float c = std::cos(rad), s = std::sin(rad);
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

transform &transform::shear(float shx, float shy)
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

quad::quad() = default;

quad::quad(double x, double y, double w, double h) : x(x), y(y), width(w), height(h)
{
}

quad quad::center(double x, double y, double w, double h)
{
    return quad(x - w * 0.5, y - h * 0.5, w, h);
}

quad quad::corner(double x, double y, double w, double h)
{
    return quad(x, y, w, h);
}

quad quad::intersection_of(const quad &q1, const quad &q2)
{
    double xc = std::max(q1.corner_x(), q2.corner_x());
    double yc = std::max(q1.corner_y(), q2.corner_y());
    double xc1 = std::min(q1.prom_x(), q2.prom_x());
    double yc1 = std::min(q1.prom_y(), q2.prom_y());
    return corner(xc, yc, xc1 - xc, yc1 - yc);
}

bool quad::intersect(const quad &q1, const quad &q2)
{
    return intersection_of(q1, q2).area() > 0.0;
}

quad &quad::translate(double nx, double ny)
{
    x += nx;
    y += ny;
    return *this;
}

quad &quad::locate_center(double nx, double ny)
{
    x = nx - width * 0.5;
    y = ny - height * 0.5;
    return *this;
}

quad &quad::locate_corner(double nx, double ny)
{
    x = nx;
    y = ny;
    return *this;
}

quad &quad::resize(double nw, double nh)
{
    width = nw;
    height = nh;
    return *this;
}

quad &quad::inflate(double nw, double nh)
{
    width += nw;
    height += nh;
    x -= nw * 0.5;
    y -= nh * 0.5;
    return *this;
}

quad &quad::scale(double sw, double sh)
{
    width *= sw;
    height *= sh;
    return *this;
}

double quad::center_x() const
{
    return x + width * 0.5;
}

double quad::center_y() const
{
    return y + height * 0.5;
}

double quad::corner_x() const
{
    return x;
}

double quad::corner_y() const
{
    return y;
}

double quad::prom_x() const
{
    return x + width;
}

double quad::prom_y() const
{
    return y + height;
}

double quad::area() const
{
    return width * height;
}

} // namespace flux
