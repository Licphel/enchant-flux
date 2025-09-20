#pragma once
#include <kernel/hio.h>
#include <string>
#include <gfx/color.h>
#include <math/vec.h>
#include <math/mat.h>

namespace flux
{

enum shader_attrib_size
{
    FX_SHADER_1,
    FX_SHADER_2,
    FX_SHADER_4
};

struct shader_attrib
{
    unsigned int __attrib_id = 0;
    shader_attrib(unsigned int id) : __attrib_id(id)
    {
    }

    void layout(shader_attrib_size size, int components, int stride, int offset, bool normalize = false);
};

struct shader_uniform
{
    unsigned int __uniform_id = 0;
    shader_uniform(unsigned int id) : __uniform_id(id)
    {
    }

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
    void (*callback_setup)(shared<shader_program> program) = nullptr;

    ~shader_program();
    shader_attrib get_attrib(const std::string &name);
    shader_attrib get_attrib(int index);
    shader_uniform get_uniform(const std::string &name);
};

enum builtin_shader_type
{
    FX_BUILTIN_SHADER_TEXTURED,
    FX_BUILTIN_SHADER_COLORED
};

shared<shader_program> make_program(const std::string &vert, const std::string &frag,
                                    void (*callback_setup)(shared<shader_program> program));
shared<shader_program> make_program(builtin_shader_type type);

} // namespace flux