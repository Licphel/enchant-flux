#include <gfx/buf.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <memory>

namespace flux
{

void complex_buffer::end_quad()
{
    new_index(6);
    new_vertex(4);

    unsigned int k = vertex_count - 4;
    index_buf.push_back(0 + k);
    index_buf.push_back(1 + k);
    index_buf.push_back(3 + k);
    index_buf.push_back(1 + k);
    index_buf.push_back(2 + k);
    index_buf.push_back(3 + k);
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
    index_buf.clear();
    vertex_count = 0;
    index_count = 0;
    dirty = true;
}

shared<complex_buffer> make_buffer()
{
    return std::make_shared<complex_buffer>();
}

} // namespace flux