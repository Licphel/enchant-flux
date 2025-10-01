#pragma once
#include <core/def.h>
#include <gfx/image.h>

namespace flux::gfx
{

struct atlas
{
    int width;
    int height;
    byte *pixels;
    shared<image> output_image = nullptr;
    shared<texture> output_texture = nullptr;

    struct _impl;
    unique<_impl> __p;

    atlas(int w, int h);
    ~atlas();

    void begin();
    void end();
    shared<texture> accept(shared<image> image);
    void imgcpy(shared<image> image, int dest_x, int dest_y);
};

shared<atlas> make_atlas(int w, int h);

} // namespace flux