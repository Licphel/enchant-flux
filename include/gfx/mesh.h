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
    brush* begin();
    void end();
    void draw(brush* gbrush);
};

unique<mesh> make_mesh();

} // namespace flux