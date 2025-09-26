#include <gfx/shader.h>
#include <kernel/def.h>
#include <memory>
#include <gl/glew.h>
#include <kernel/log.h>

namespace flux
{

void shader_attrib::layout(shader_vertex_data_type size, int components, int stride, int offset, bool normalize)
{
    glEnableVertexAttribArray(__attrib_id);
    GLenum type;
    switch (size)
    {
    case FX_VDAT_BYTE:
        type = GL_UNSIGNED_BYTE;
        break;
    case FX_VDAT_INT:
        type = GL_UNSIGNED_SHORT;
        break;
    case FX_VDAT_HALF_FLOAT:
        type = GL_HALF_FLOAT;
        break;
    case FX_VDAT_FLOAT:
        type = GL_FLOAT;
        break;
    default:
        type = GL_UNSIGNED_BYTE;
        break;
    }
    glVertexAttribPointer(__attrib_id, components, type, normalize, stride, reinterpret_cast<void *>(offset));
}

void shader_uniform::set_texture_unit(int unit)
{
    glUniform1i(__uniform_id, unit);
}

void shader_uniform::seti(int v)
{
    glUniform1i(__uniform_id, v);
}

void shader_uniform::set(double v)
{
    glUniform1f(__uniform_id, v);
}

void shader_uniform::set(const vec2 &v)
{
    glUniform2f(__uniform_id, v.x, v.y);
}

void shader_uniform::set(const vec3 &v)
{
    glUniform3f(__uniform_id, v.x, v.y, v.z);
}

void shader_uniform::set(const color &v)
{
    glUniform4f(__uniform_id, v.r, v.g, v.b, v.a);
}

void shader_uniform::set(const transform &v)
{
    float m[16] = {v.m00, v.m10, 0.0f, 0.0f, v.m01, v.m11, 0.0f, 0.0f,
                   0.0f,  0.0f,  1.0f, 0.0f, v.m02, v.m12, 0.0f, 1.0f};
    glUniformMatrix4fv(__uniform_id, 1, GL_FALSE, m);
}

shader_program::~shader_program()
{
    glDeleteProgram(__program_id);
}

shader_attrib shader_program::get_attrib(const std::string &name)
{
    return {glGetAttribLocation(__program_id, name.c_str())};
}

shader_attrib shader_program::get_attrib(int index)
{
    return {index};
}

shader_uniform shader_program::get_uniform(const std::string &name)
{
    return {glGetUniformLocation(__program_id, name.c_str())};
}

shader_uniform shader_program::cache_uniform(const std::string &name)
{
    auto uni = get_uniform(name);
    cached_uniforms.push_back(uni);
    return uni;
}

static int __build_shader_part(std::string source, GLenum type)
{
    int id = glCreateShader(type);

    if (id == 0)
        prtlog_throw(FATAL, "Fail to create shader.");

    const char *src = source.c_str();
    GLint len = static_cast<GLint>(source.size());

    glShaderSource(id, 1, &src, &len);
    glCompileShader(id);

    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(id, sizeof(log), nullptr, log);
        glDeleteShader(id);
        prtlog_throw(FATAL, "glsl compile error: {}", std::string(log));
    }

    return id;
}

shared<shader_program> make_program(const std::string &vert, const std::string &frag,
                                    void (*callback_setup)(shared<shader_program> program))
{
    shared<shader_program> program = std::make_shared<shader_program>();
    int id = program->__program_id = glCreateProgram();
    int vert_id = __build_shader_part(vert, GL_VERTEX_SHADER);
    int frag_id = __build_shader_part(frag, GL_FRAGMENT_SHADER);

    glAttachShader(id, vert_id);
    glAttachShader(id, frag_id);
    glBindFragDataLocation(id, 0, "fragColor");
    glLinkProgram(id);
    glDetachShader(id, vert_id);
    glDetachShader(id, frag_id);
    glValidateProgram(id);
    glDeleteShader(vert_id);
    glDeleteShader(frag_id);

    program->callback_setup = callback_setup;

    return program;
}

static const std::string __dvert_textured = "#version 330 core\n"
                                            "layout(location = 0) in vec2 i_position;\n"
                                            "layout(location = 1) in vec4 i_color;\n"
                                            "layout(location = 2) in vec2 i_texCoord;\n"
                                            "out vec4 o_color;\n"
                                            "out vec2 o_texCoord;\n"
                                            "uniform mat4 u_proj;\n"
                                            "void main() {\n"
                                            "    o_color = i_color;\n"
                                            "    o_texCoord = i_texCoord;\n"
#ifdef FX_Y_IS_DOWN
                                            "    gl_Position = u_proj * vec4(i_position.x, -i_position.y, 0.0, 1.0);\n"
#else
                                            "    gl_Position = u_proj * vec4(i_position.x, i_position.y, 0.0, 1.0);\n"
#endif
                                            "}";

static const std::string __dfrag_textured = "#version 330 core\n"
                                            "in vec4 o_color;\n"
                                            "in vec2 o_texCoord;\n"
                                            "out vec4 fragColor;\n"
                                            "uniform sampler2D u_tex;\n"
                                            "void main() {\n"
                                            "    fragColor = o_color * texture(u_tex, o_texCoord);\n"
                                            "}";

static const std::string __dvert_colored = "#version 330 core\n"
                                           "layout(location = 0) in vec2 i_position;\n"
                                           "layout(location = 1) in vec4 i_color;\n"
                                           "out vec4 o_color;\n"
                                           "uniform mat4 u_proj;\n"
                                           "void main() {\n"
                                           "    o_color = i_color;\n"
#ifdef FX_Y_IS_DOWN
                                           "    gl_Position = u_proj * vec4(i_position.x, -i_position.y, 0.0, 1.0);\n"
#else
                                           "    gl_Position = u_proj * vec4(i_position.x, i_position.y, 0.0, 1.0);\n"
#endif
                                           "}";

static const std::string __dfrag_colored = "#version 330 core\n"
                                           "in vec4 o_color;\n"
                                           "out vec4 fragColor;\n"
                                           "void main() {\n"
                                           "    fragColor = o_color;\n"
                                           "}";

static shared<shader_program> __builtin_colored = nullptr, __builtin_textured = nullptr;

shared<shader_program> make_program(builtin_shader_type type)
{
    if (__builtin_colored == nullptr || __builtin_textured == nullptr)
    {
        __builtin_colored = make_program(__dvert_colored, __dfrag_colored, [](shared<shader_program> program) {
            program->get_attrib(0).layout(FX_VDAT_FLOAT, 2, 16, 0);
            program->get_attrib(1).layout(FX_VDAT_HALF_FLOAT, 4, 16, 8);

            if (program->cached_uniforms.size() > 0)
                return;
            program->cache_uniform("u_proj"); // 0
        });
        __builtin_textured = make_program(__dvert_textured, __dfrag_textured, [](shared<shader_program> program) {
            program->get_attrib(0).layout(FX_VDAT_FLOAT, 2, 24, 0);
            program->get_attrib(1).layout(FX_VDAT_HALF_FLOAT, 4, 24, 8);
            program->get_attrib(2).layout(FX_VDAT_FLOAT, 2, 24, 16);

            if (program->cached_uniforms.size() > 0)
                return;
            program->cache_uniform("u_proj");                    // 0
            program->cache_uniform("u_tex").set_texture_unit(1); // 1
        });
    }
    switch (type)
    {
    case FX_BUILTIN_SHADER_COLORED:
        return __builtin_colored;
    case FX_BUILTIN_SHADER_TEXTURED:
        return __builtin_textured;
    }
    return nullptr;
}

} // namespace flux
