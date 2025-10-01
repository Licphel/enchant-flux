#include <gfx/font.h>
#include <gfx/atlas.h>
#include <core/log.h>
#include <cuchar>
#include <string>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

namespace flux::gfx
{

struct font::_impl
{
    FT_FaceRec_ *face_ptr;
    FT_LibraryRec_ *lib_ptr;
    std::map<int, shared<atlas>> codemap;
    double res, pix;
};

// for unique_ptr<_impl> to refer
font::font() : __p(std::make_unique<_impl>())
{
}

// for unique_ptr<_impl> to refer
font::~font()
{
    FT_Done_Face(__p->face_ptr);
    FT_Done_FreeType(__p->lib_ptr);
}

shared<texture> __flush_codemap(font::_impl *__p, general_char ch, shared<image> img)
{
    int code = (int)floor(static_cast<int>(ch) / 256.0);
    if (__p->codemap.find(code) == __p->codemap.end())
    {
        __p->codemap[code] = std::make_shared<atlas>(512, 512);
        __p->codemap[code]->begin();
    }

    auto atl = __p->codemap[code];
    auto tptr = atl->accept(img);
    atl->end();
    return tptr;
}

glyph font::make_glyph(general_char ch)
{
    auto face = __p->face_ptr;
    unsigned int idx = FT_Get_Char_Index(face, ch);

    FT_Set_Pixel_Sizes(face, 0, __p->res);
    FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Bitmap_ m0 = face->glyph->bitmap;

    int len = (int)(m0.width * m0.rows * 4);
    byte *buf = new byte[len];
    for (int i = 0; i < len; i += 4)
    {
        byte grey = m0.buffer[i / 4];
        buf[i + 0] = 255;
        buf[i + 1] = 255;
        buf[i + 2] = 255;
        buf[i + 3] = grey;
    }

    shared<image> img = make_image((int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows, buf);

    double ds = __p->res / __p->pix;

    glyph g;
    g.texpart = __flush_codemap(__p.get(), ch, img);
    g.size.x = face->glyph->metrics.width / ds / 64.0;
    g.size.y = face->glyph->metrics.height / ds / 64.0;
    g.advance = face->glyph->advance.x / ds / 64.0;
    g.offset.x = face->glyph->metrics.horiBearingX / ds / 64.0;
#ifdef FX_Y_IS_DOWN
    g.offset.y = -face->glyph->metrics.horiBearingY / ds / 64.0 + f_height + face->bbox.yMin / ds / 64.0;
#else
    g.offset.y = face->glyph->metrics.horiBearingY / ds / 64.0 - g.size.y;
#endif

    return g;
}

static std::u32string to_u32(std::string_view u8)
{
    std::u32string out;
    out.reserve(u8.size());
    for (auto it = u8.begin(); it != u8.end();)
    {
        general_char cp = 0;
        unsigned char c = *it++;
        if (c < 0x80)
            cp = c;
        else if ((c & 0xE0) == 0xC0)
            cp = (c & 0x1F) << 6 | (*it++ & 0x3F);
        else if ((c & 0xF0) == 0xE0)
            cp = (c & 0x0F) << 12 | (*it++ & 0x3F) << 6 | (*it++ & 0x3F);
        else if ((c & 0xF8) == 0xF0)
            cp = (c & 0x07) << 18 | (*it++ & 0x3F) << 12 | (*it++ & 0x3F) << 6 | (*it++ & 0x3F);
        out.push_back(cp);
    }
    return out;
}

quad font::make_vtx(brush *brush, const std::string &u8_str, double x, double y, double scale, double max_w)
{
    const std::u32string str = to_u32(u8_str);

    if (str.length() == 0 || str.length() > INT16_MAX)
        return {};

    double h_scaled = scale * f_lspc;
    double w = 0;
    double lw = 0;
    double h = 0;
    double lh = 0;
    double dx = x;
    double dy = y;
    bool endln = false;

    for (int i = 0; i < str.length(); i++)
    {
        general_char ch = str[i];

        if (ch == '\n' || endln)
        {
#ifdef FX_Y_IS_DOWN
            dy += h_scaled;
#else
            dy -= h_scaled;
#endif
            dx = x;
            endln = false;
            w = std::max(w, lw);
            lw = 0;
            h += h_scaled;
            lh = 0;
            continue;
        }

        glyph g = get_glyph(ch) * scale;

        if (dx - x + g.advance >= max_w)
        {
            endln = true;
            i -= 1;
            i = std::max(i, -1);
            continue;
        }

        lh = std::max(g.size.y, lh);
        lw += g.advance;

        if (brush != nullptr)
            brush->draw_texture(g.texpart, {dx + g.offset.x, dy + g.offset.y, g.size.x, g.size.y});
        dx += g.advance;

        if (i == str.length() - 1 || (get_glyph(str[i + 1]) * scale).advance + dx - x >= max_w)
            lw += g.size.x - g.advance;
    }

#ifdef FX_Y_IS_DOWN
    return {x, y, w, h + lh};
#else
    return {x, dy, w, h + lh};
#endif
}

shared<font> load_font(const hio_path &path, double res_h, double pixel_h)
{
    auto fptr = std::make_shared<font>();

    FT_LibraryRec_ *lib;
    FT_FaceRec_ *face;
    FT_Init_FreeType(&lib);
    FT_New_Face(lib, path.absolute.c_str(), 0, &face);
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);

    fptr->__p->face_ptr = face;
    fptr->__p->lib_ptr = lib;
    fptr->__p->res = res_h;
    fptr->__p->pix = pixel_h;
    fptr->f_height = pixel_h;
    fptr->f_ascend = face->ascender / 64.0;
    fptr->f_descend = face->descender / 64.0;
    fptr->f_lspc = pixel_h + 1;

    return fptr;
}

} // namespace flux::gfx
