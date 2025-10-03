#pragma once
#include <core/hio.h>
#include <string>
#include <gfx/color.h>
#include <core/math.h>
#include <core/math.h>
#include <functional>

namespace flux::gfx
{

enum shader_vertex_data_type
{
    FX_VDAT_BYTE,
    FX_VDAT_INT,
    FX_VDAT_HALF_FLOAT,
    FX_VDAT_FLOAT
};

struct shader_attrib
{
    unsigned int __attrib_id = 0;

    shader_attrib(unsigned int id);

    void layout(shader_vertex_data_type size, int components, int stride, int offset, bool normalize = false);
};

struct shader_uniform
{
    unsigned int __uniform_id = 0;
    
    shader_uniform(unsigned int id);

    // bind the uniform to a texture unit, and use #bind_texture to actually bind it.
    void set_texture_unit(int unit);
    void seti(int v);
    void set(double v);
    void set(const vec2 &v);
    void set(const vec3 &v);
    void set(const color &v);
    void set(const transform &v);
};

struct shader_program
{
    /* unstable */ unsigned int __program_id = 0;
    std::function<void (shared<shader_program> program)> callback_setup;
    /* unstable */ bool __has_setup = false;
    std::vector<shader_uniform> cached_uniforms;

    ~shader_program();
    shader_attrib get_attrib(const std::string &name);
    shader_attrib get_attrib(int index);
    shader_uniform get_uniform(const std::string &name);
    shader_uniform cache_uniform(const std::string &name);
};

enum builtin_shader_type
{
    FX_BUILTIN_SHADER_TEXTURED,
    FX_BUILTIN_SHADER_COLORED
};

shared<shader_program> make_program(const std::string &vert, const std::string &frag,
                                    std::function<void (shared<shader_program> program)> callback_setup);
shared<shader_program> make_program(builtin_shader_type type);

} // namespace flux