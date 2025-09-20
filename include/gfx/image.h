#pragma once
#include <kernel/hio.h>
#include <kernel/def.h>
#include <math/quad.h>

namespace flux
{

struct image
{
    int width, height;
    byte *pixels;
    /* unstable */ bool __is_from_stb = false;

    image() = default;
    image(byte *data, int w, int h) : width(w), height(h), pixels(data)
    {
    }
    ~image();
};

struct texture
{
    int u = 0;
    int v = 0;
    int width = 0;
    int height = 0;
    int fwidth = 0;
    int fheight = 0;
    /* maybe nullptr */ shared<image> __relying_image = nullptr;
    /* unstable */ unsigned int __texture_id = 0;
    bool __is_framebuffer;
    shared<texture> root = nullptr;

    ~texture();
};

shared<image> load_image(const hpath &path);
shared<image> make_image(int width, int height, byte *data);
shared<texture> make_texture(shared<image> img);
void lazylink_texture_data(shared<texture> tex, shared<image> img);
shared<texture> cut_texture(shared<texture> tex, const quad &src);
void bind_texture(int i, shared<texture> tex);

} // namespace flux