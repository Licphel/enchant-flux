#include <gfx/mesh.h>

namespace flux
{

mesh::mesh()
{
    buffer = make_buffer();
    brush_binded = make_brush(buffer);
}

brush *mesh::retry()
{
    buffer->clear();
    brush_binded->__is_in_mesh = true;
    return brush_binded.get();
}

void mesh::record()
{
    m_state = brush_binded->m_state;
    brush_binded->__is_in_mesh = false;
}

void mesh::draw(brush *gbrush)
{
    gbrush->flush();
    auto old_state = gbrush->m_state;
    auto old_buf = gbrush->buffer;
    gbrush->use(m_state);
    gbrush->buffer = buffer;
    gbrush->flush();
    gbrush->buffer = old_buf;
    gbrush->use(old_state);
}

shared<mesh> make_mesh()
{
    return std::make_unique<mesh>();
}

} // namespace flux
