#include <gfx/color.h>

namespace flux::gfx
{

color color::from_bytes(byte r, byte g, byte b, byte a)
{
    return color(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

void color::get_bytes(byte *br, byte *bg, byte *bb, byte *ba)
{
    *br = (byte)(255 * r);
    *bg = (byte)(255 * g);
    *bb = (byte)(255 * b);
    *ba = (byte)(255 * a);
}

color color::operator+(double s) const
{
    return color(r + s, g + s, b + s, a);
}

color color::operator-(double s) const
{
    return color(r - s, g - s, b - s, a);
}

color color::operator/(double s) const
{
    return color(r / s, g / s, b / s, a);
}

} // namespace flux::gfx