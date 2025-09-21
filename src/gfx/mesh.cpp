#include <gfx/mesh.h>
#include <gl/glew.h>
#include <gl/gl.h>

namespace flux
{

mesh::mesh()
{
    buffer = make_buffer();
    brush_binded = make_brush(buffer);
    brush_binded->__mesh_root = this;
}

mesh::~mesh()
{
    glDeleteVertexArrays(1, &__vao);
    glDeleteBuffers(1, &__vbo);
    glDeleteBuffers(1, &__ebo);
}

brush *mesh::retry()
{
    buffer->clear();
    if (!__is_direct)
        brush_binded->__is_in_mesh = true;
    return brush_binded.get();
}

void mesh::record()
{
    m_state = brush_binded->m_state;
    if (!__is_direct)
        brush_binded->__is_in_mesh = false;
}

void mesh::draw(brush *gbrush)
{
    auto old_state = gbrush->m_state;
    auto old_buf = gbrush->buffer;
    auto old_msh = gbrush->__mesh_root;

    gbrush->use(m_state);
    gbrush->buffer = buffer;
    gbrush->__mesh_root = this;
    gbrush->__clear_when_flush = false;

    gbrush->flush();

    gbrush->__clear_when_flush = true;
    gbrush->__mesh_root = old_msh;
    gbrush->buffer = old_buf;
    gbrush->use(old_state);
}

shared<mesh> make_mesh()
{
    shared<mesh> msh = std::make_unique<mesh>();
    shared<complex_buffer> buf = msh->buffer;

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    msh->__vao = vao;
    msh->__vbo = vbo;
    msh->__ebo = ebo;

    return msh;
}

} // namespace flux
