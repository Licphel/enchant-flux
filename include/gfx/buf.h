#pragma once
#include <cstring>
#include <gfx/image.h>
#include <gfx/shader.h>
#include <memory>
#include <vector>

namespace flux
{

enum graph_mode
{
    FX_COLORED_POINT = 0,
    FX_COLORED_LINE = 1,
    FX_COLORED_TRIANGLE = 2,
    FX_COLORED_QUAD = 3,

    FX_TEXTURED_QUAD = 16,
};

// currently it only supports quad-drawing indexing.
// maybe in the future I'll extend it.
struct complex_buffer
{
    std::vector<byte> vertex_buf;
    std::vector<unsigned int> index_buf;
    int vertex_count = 0;
    int index_count = 0;
    bool dirty;
    bool __cap_changed;

    /* unstable */ unsigned int __vao, __vbo, __ebo;

    ~complex_buffer();

    template <typename T> complex_buffer &operator<<(const T &t)
    {
        std::size_t s = sizeof(t);
        std::size_t old = vertex_buf.size();
        std::size_t oldcap = vertex_buf.capacity();
        vertex_buf.resize(old + s);
        if (vertex_buf.capacity() != oldcap)
            __cap_changed = true;

        std::memcpy(vertex_buf.data() + old, &t, s);
        dirty = true;

        return *this;
    }

    void new_vertex(int count);
    void new_index(int count);
    void end_quad();
    void clear();
};

struct graph_state
{
    graph_mode mode = FX_TEXTURED_QUAD;
    shared<texture> texture = nullptr;
    shared<shader_program> program = nullptr;
    void (*callback_uniform)(shared<shader_program> program) = nullptr;
    void (*callback_buffer_append)(unique<complex_buffer> buf) = nullptr;
};

shared<complex_buffer> make_buffer();

} // namespace flux