#pragma once
#include <cstring>
#include <gfx/image.h>
#include <gfx/brush.h>
#include <gfx/buf.h>
#include <memory>
#include <vector>

namespace flux
{

struct mesh
{
    graph_state m_state;
    shared<complex_buffer> buffer;
    shared<brush> brush_binded;

    mesh();
    brush *retry();
    void record();
    void draw(brush *gbrush);
};

shared<mesh> make_mesh();

} // namespace flux