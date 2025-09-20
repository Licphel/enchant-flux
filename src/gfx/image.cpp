#include <gfx/image.h>
#include <kernel/common.h>
#include <gl/glew.h>
#include <gl/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace flux
{

image::~image()
{
    if (__is_from_stb)
        stbi_image_free(pixels);
    else
        delete[] pixels;
    pixels = nullptr;
}

shared<image> load_image(const hpath &path)
{
    shared<image> img = std::make_shared<image>();
    img->pixels = stbi_load(path.absolute.c_str(), &img->width, &img->height, nullptr, 4);
    if (img->pixels == nullptr)
        prtlog_throw(FATAL, "path not found: {}", path.absolute);
    img->__is_from_stb = true;
    return img;
}

shared<image> make_image(int width, int height, byte *data)
{
    shared<image> img = std::make_shared<image>();
    img->width = width;
    img->height = height;
    img->pixels = data;
    img->__is_from_stb = false;
    return img;
}

texture::~texture()
{
    if (root == nullptr)
        glDeleteTextures(1, &__texture_id);
}

shared<texture> make_texture(shared<image> img)
{
    shared<texture> tex = std::make_shared<texture>();
    unsigned int id;
    glGenTextures(1, &id);
    tex->__texture_id = id;

    if (img != nullptr)
        lazylink_texture_data(tex, img);

    return tex;
}

void lazylink_texture_data(shared<texture> tex, shared<image> img)
{
    tex->__relying_image = img;
    tex->fwidth = tex->width = img->width;
    tex->fheight = tex->height = img->height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->__texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

shared<texture> cut_texture(shared<texture> tex, const quad &src)
{
    shared<texture> ntex = std::make_shared<texture>();
    ntex->width = src.width;
    ntex->height = src.height;
    ntex->fwidth = tex->fwidth;
    ntex->fheight = tex->fheight;
    ntex->u = src.x;
    ntex->v = src.y;
    ntex->root = tex;
    ntex->__relying_image = tex->__relying_image;
    ntex->__texture_id = tex->__texture_id;
    ntex->__is_framebuffer = tex->__is_framebuffer;
    return ntex;
}

void bind_texture(int unit, shared<texture> tex)
{
    if (unit == 0)
        prtlog_throw(FATAL, "cannot bind to texture unit 0, since it is reserved.");
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex->__texture_id);
}

} // namespace flux