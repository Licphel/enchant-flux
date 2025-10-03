#pragma once
#include <cstring>
#include <gfx/image.h>
#include <gfx/shader.h>
#include <memory>
#include <vector>

namespace flux::gfx
{

enum graph_mode
{
    FX_COLORED_POINT = 0,
    FX_COLORED_LINE = 1,
    FX_COLORED_TRIANGLE = 2,
    FX_COLORED_QUAD = 3,

    //FX_TEXTURED_TRIANGLE = 16, <-- not implemented yet
    FX_TEXTURED_QUAD = 17
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
    bool __icap_changed;
    bool __vcap_changed;

    // write a vertex, generally, T is float.
    template <typename T> complex_buffer & vtx(T t)
    {
        size_t s = sizeof(t);
        size_t old = vertex_buf.size();
        if(old + s > vertex_buf.capacity())
        {
            vertex_buf.reserve(vertex_buf.capacity() * 2);
            __vcap_changed = true;
        }
    
        vertex_buf.resize(old + s);
        std::memcpy(vertex_buf.data() + old, &t, s);
        dirty = true;

        return *this;
    }

    // write an index.
    inline complex_buffer & idx(unsigned int t)
    {
        size_t s = sizeof(t);
        size_t old = index_buf.size();
        if(old + s > index_buf.capacity())
        {
            index_buf.reserve(index_buf.capacity() * 2);
            __icap_changed = true;
        }
    
        index_buf.push_back(t);
        dirty = true;

        return *this;
    }

    void new_vertex(int count);
    void new_index(int count);
    void end_quad();
    void clear();
};

shared<complex_buffer> make_buffer();

} // namespace flux