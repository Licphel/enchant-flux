#include <gfx/atlas.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <kernel/common.h>

namespace flux
{

void image_atlas::begin()
{
    root = new atlas_node(0, 0, width, height);
    output_image = make_image(width, height, pixels);
    output_texture = make_texture(nullptr);

    //pre-set
    output_texture->fwidth = output_image->width = width;
    output_texture->fheight = output_image->height = height;
}

void image_atlas::end()
{
    lazylink_texture_data(output_texture, output_image);
    free_node(root);
}

shared<texture> image_atlas::accept(shared<image> image)
{
    if (image == nullptr || image->pixels == nullptr)
        return nullptr;

    int requested_width = image->width;
    int requested_height = image->height;

    atlas_node *node = find_node(root, requested_width, requested_height);
    if (!node)
        prtlog_throw(FATAL, "atlas is not big enough. please expand it.");

    split_node(node, requested_width, requested_height);
    node->used = true;

    int insert_x = node->x + padding;
    int insert_y = node->y + padding;

    copy_image_to_atlas(image, insert_x, insert_y);

    return cut_texture(output_texture, {insert_x, insert_y, image->width, image->height});
}

image_atlas::atlas_node *image_atlas::find_node(atlas_node *node, int w, int h)
{
    if (node->used)
    {
        atlas_node *right_node = find_node(node->right, w, h);
        if (right_node)
            return right_node;
        return find_node(node->down, w, h);
    }
    else if (w <= node->width && h <= node->height)
        return node;
    return nullptr;
}

void image_atlas::split_node(atlas_node *node, int w, int h)
{
    node->right = new atlas_node(node->x + w, node->y, node->width - w, node->height);
    node->down = new atlas_node(node->x, node->y + h, w, node->height - h);
}

void image_atlas::copy_image_to_atlas(shared<image> image, int dest_x, int dest_y)
{
    // suppose channels are the same.
    // (rgba format)
    for (int y = 0; y < image->height; ++y)
    {
        for (int x = 0; x < image->width; ++x)
        {
            int src_index = (y * image->width + x) * 4;
            int dest_index = ((dest_y + y) * width + (dest_x + x)) * 4;

            for (int c = 0; c < 4; ++c)
                pixels[dest_index + c] = image->pixels[src_index + c];
        }
    }
}

void image_atlas::free_node(atlas_node *node)
{
    if (!node)
        return;
    free_node(node->right);
    free_node(node->down);
    delete node;
}

shared<image_atlas> make_image_atlas(int w, int h, int padding)
{
    return std::make_shared<image_atlas>(w, h, padding);
}

} // namespace flux