#pragma once
#include <core/math.h>
#include <core/math.h>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <map>

namespace flux::gfx
{

enum font_render_align
{
    FX_FONT_LEFT_ALIGN = 1LL << 0,
    FX_FONT_RIGHT_ALIGN = 1LL << 1,
    FX_FONT_H_CENTER_ALIGN = 1LL << 2,
    FX_FONT_UP_ALIGN = 1LL << 3,
    FX_FONT_DOWN_ALIGN = 1LL << 4,
    FX_FONT_V_CENTER_ALIGN = 1LL << 5
};

struct glyph
{
    shared<texture> texpart;
    vec2 size;
    double advance;
    vec2 offset;

    glyph operator*(double scl)
    {
        return {
            texpart,
            size * scl,
            advance * scl,
            offset * scl,
        };
    }
};

struct font
{
    std::map<general_char, glyph> glyph_map;
    double f_height = 0;
    double f_lspc = 0;
    double f_ascend = 0;
    double f_descend = 0;

    struct _impl;
    unique<_impl> __p;

    font();
    ~font();

    glyph get_glyph(general_char ch)
    {
        if (glyph_map.find(ch) == glyph_map.end())
            return glyph_map[ch] = make_glyph(ch);
        return glyph_map[ch];
    }

    glyph make_glyph(general_char ch);
    quad make_vtx(brush *brush, const std::string &str, double x, double y, double scale = 1, double max_w = INT_MAX);
};

shared<font> load_font(const hio_path &path, double res_h, double pixel_h);

} // namespace flux::gfx
