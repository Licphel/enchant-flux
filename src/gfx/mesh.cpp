#include <gfx/mesh.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <memory>

namespace flux
{

void complex_buffer::init_quad()
{
    for (unsigned int i = 0, k = 0;; i += 6, k += 4)
    {
        if (index_buf.size() + 6 > index_buf.capacity())
            break;
        index_buf.push_back(0 + k);
        index_buf.push_back(1 + k);
        index_buf.push_back(3 + k);
        index_buf.push_back(1 + k);
        index_buf.push_back(2 + k);
        index_buf.push_back(3 + k);
    }
}

void complex_buffer::new_vertex(int count)
{
    vertex_count += count;
}

void complex_buffer::new_index(int count)
{
    index_count += count;
}

void complex_buffer::clear()
{
    vertex_buf.clear();
    vertex_count = index_count = 0;
    dirty = true;
}

complex_buffer::complex_buffer(int size_in_bytes)
{
    vertex_buf.reserve(size_in_bytes);
    index_buf.reserve(size_in_bytes / 2 * 3);
}

complex_buffer::~complex_buffer()
{
    glDeleteVertexArrays(1, &__vao);
    glDeleteBuffers(1, &__vbo);
    glDeleteBuffers(1, &__ebo);
}

mesh::mesh(mesh_state m, int size_in_bytes) : m_state(m)
{
    buffer = std::make_shared<complex_buffer>(size_in_bytes);
}

shared<complex_buffer> make_buffer(int size_in_bytes)
{
    shared<complex_buffer> buf = std::make_shared<complex_buffer>(size_in_bytes);
    buf->init_quad();

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    buf->__vao = vao;
    buf->__vbo = vbo;
    buf->__ebo = ebo;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, buf->vertex_buf.capacity(), buf->vertex_buf.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->index_buf.capacity(), buf->index_buf.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return buf;
}

} // namespace flux