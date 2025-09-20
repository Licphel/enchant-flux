#pragma once
#include <kernel/def.h>
#include <gfx/image.h>

namespace flux
{

struct image_atlas
{
    int width;
    int height;
    byte *pixels;
    shared<image> output_image = nullptr;
    shared<texture> output_texture = nullptr;

    struct atlas_node
    {
        int x, y;
        int width, height;
        bool used;
        atlas_node *right;
        atlas_node *down;

        atlas_node(int x, int y, int w, int h)
            : x(x), y(y), width(w), height(h), used(false), right(nullptr), down(nullptr)
        {
        }
    };

    atlas_node *root = nullptr;
    int padding = 0;

    image_atlas(int w, int h, int padding = 0) : width(w), height(h), pixels(new byte[w * h * 4]), padding(padding)
    {
    }

    void begin();
    void end();
    shared<texture> accept(shared<image> image);

  private:
    atlas_node *find_node(atlas_node *node, int w, int h);
    void split_node(atlas_node *node, int w, int h);
    void copy_image_to_atlas(shared<image> image, int dest_x, int dest_y);
    void free_node(atlas_node *node);
};

shared<image_atlas> make_image_atlas(int w, int h, int padding = 0);

} // namespace flux