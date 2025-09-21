#pragma once
#include <math/vec.h>
#include <math/quad.h>

namespace flux
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

struct font
{
    /* unstable */ unsigned int __font_id;
};

double ft_height(font f);
quad ft_area(font f, double x, double y, font_render_align align);
    
} // namespace flux

