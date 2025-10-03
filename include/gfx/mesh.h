#pragma once
#include <cstring>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <gfx/cbuf.h>
#include <memory>
#include <vector>

namespace flux::gfx
{

struct mesh
{
    graph_state m_state;
    shared<complex_buffer> buffer;
    shared<brush> brush_binded;

    /* unstable */ unsigned int __vao, __vbo, __ebo;
    /* unstable */ bool __is_direct;

    mesh();
    ~mesh();
    // clear the mesh and attempt to redraw it.
    brush *retry();
    // record the mesh state and buffer content, and end the drawing.
    void record();
    // draw the mesh with the brush. the brush should be direct-to-screen.
    void draw(brush *gbrush);
};

shared<mesh> make_mesh();

} // namespace flux