#include <gfx/brush.h>
#include <gfx/toolkit.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <memory>
#include <kernel/common.h>

namespace flux
{

static uint16_t __to_half(float f)
{
    union {
        float f;
        uint32_t u;
    } v = {f};
    uint32_t s = (v.u >> 31) & 0x1;
    uint32_t e = (v.u >> 23) & 0xFF;
    uint32_t m = v.u & 0x7FFFFF;
    if (e == 0xFF)
        return uint16_t((s << 15) | 0x7C00 | (m ? 1 : 0));
    if (!e)
        return uint16_t((s << 15) | (m >> 13));
    int32_t E = int32_t(e) - 127 + 15;
    if (E > 31)
        E = 31;
    if (E < 0)
        E = 0;
    return uint16_t((s << 15) | (E << 10) | (m >> 13));
}

void __w_half(shared<complex_buffer> buf, const color &col)
{
    *buf << __to_half(col.r) << __to_half(col.g) << __to_half(col.b) << __to_half(col.a);
}

brush::brush()
{
    cl_norm();
    ts_push();
    __default_colored = make_program(FX_BUILTIN_SHADER_COLORED);
    __default_textured = make_program(FX_BUILTIN_SHADER_TEXTURED);
}

shared<complex_buffer> brush::lock_buffer()
{
    if (buffer.expired())
        prtlog_throw(FATAL, "try to paint on an expired buffer.");
    return buffer.lock();
}

graph_state &brush::current_state()
{
    return m_state;
}

void brush::cl_norm()
{
    cl_set({});
}

void brush::cl_set(const color &col)
{
    vertex_color[0] = vertex_color[1] = vertex_color[2] = vertex_color[3] = col;
}

void brush::cl_mrg(const color &col)
{
    vertex_color[0] = vertex_color[0] * col;
    vertex_color[1] = vertex_color[1] * col;
    vertex_color[2] = vertex_color[2] * col;
    vertex_color[3] = vertex_color[3] * col;
}

void brush::cl_mrg(double v)
{
    vertex_color[0] = vertex_color[0] * v;
    vertex_color[1] = vertex_color[1] * v;
    vertex_color[2] = vertex_color[2] * v;
    vertex_color[3] = vertex_color[3] * v;
}

void brush::ts_push()
{
    transform_stack.push(transform());
}

void brush::ts_pop()
{
    transform_stack.pop();
}

void brush::ts_load(const transform &t)
{
    transform_stack.push(t);
}

void brush::ts_trs(const vec2 &v)
{
    transform_stack.top().translate(v.x, v.y);
}

void brush::use(const camera &cam)
{
    flush();
    camera_binded = cam;
    viewport(cam.viewport);
}

void brush::use(shared<shader_program> program)
{
    if (m_state.program->__program_id != program->__program_id)
    {
        flush();
        m_state.program = program;
    }
}

transform brush::get_combined_transform()
{
    transform cpy = camera_binded.combined_t;
    return cpy.multiply(transform_stack.top());
}

void brush::flush()
{
    auto buf = lock_buffer();
    if (buf->vertex_buf.size() <= 0)
        return;

    // ignore empty-flushing.
    if (__is_in_mesh)
        prtlog_throw(FATAL, "it seems that somewhere the brush is flushed in a mesh. the state cannot be consistent!");

    shared<shader_program> program_used;
    if (m_state.program != nullptr && m_state.program->__program_id != 0)
        program_used = m_state.program;
    else
        switch (m_state.mode)
        {
        case FX_TEXTURED_QUAD:
            program_used = __default_textured;
            break;
        default:
            program_used = __default_colored;
            break;
        }

    glBindVertexArray(buf->__vao);
    glBindBuffer(GL_ARRAY_BUFFER, buf->__vbo);
    if (buf->dirty)
    {
        if (buf->__cap_changed)
            glBufferData(GL_ARRAY_BUFFER, buf->vertex_buf.capacity(), buf->vertex_buf.data(), GL_DYNAMIC_DRAW);
        else
            glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertex_buf.size(), buf->vertex_buf.data());
        buf->dirty = false;
    }

    glUseProgram(program_used->__program_id);
    if (program_used->callback_setup != nullptr)
        program_used->callback_setup(program_used);

    if (m_state.callback_uniform != nullptr)
        m_state.callback_uniform(program_used);
    program_used->get_uniform("u_proj").set(get_combined_transform());

    if (m_state.mode == FX_TEXTURED_QUAD || m_state.mode == FX_COLORED_QUAD)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->__ebo);
        if (buf->__cap_changed)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->index_buf.capacity(), buf->index_buf.data(), GL_STATIC_DRAW);
    }
    buf->__cap_changed = false;

    switch (m_state.mode)
    {
    case FX_TEXTURED_QUAD:
        program_used->get_uniform("u_tex").set_texture_unit(1);
        bind_texture(1, m_state.texture);
        glDrawElements(GL_TRIANGLES, buf->index_count, GL_UNSIGNED_INT, 0);
        break;
    case FX_COLORED_QUAD:
        glDrawElements(GL_TRIANGLES, buf->index_count, GL_UNSIGNED_INT, 0);
        break;
    case FX_COLORED_LINE:
        glDrawArrays(GL_LINES, 0, buf->vertex_count);
        break;
    case FX_COLORED_POINT:
        glDrawArrays(GL_POINTS, 0, buf->vertex_count);
        break;
    case FX_COLORED_TRIANGLE:
        glDrawArrays(GL_TRIANGLES, 0, buf->vertex_count);
        break;
    default:
        prtlog_throw(FATAL, "Unknown graphics mode.");
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void brush::assert_mode(graph_mode mode)
{
    if (m_state.mode != mode)
    {
        flush();
        m_state.mode = mode;
    }
}

void brush::assert_texture(shared<texture> tex)
{
    if (m_state.texture == nullptr || m_state.texture->__texture_id != tex->__texture_id)
    {
        flush();
        m_state.texture = tex;
    }
}

void brush::draw_texture(shared<texture> tex, const quad &dst, const quad &src, brush_flag flag)
{
    auto buf = lock_buffer();

    assert_mode(FX_TEXTURED_QUAD);
    assert_texture(tex);

    float u = (src.x + tex->u) / tex->fwidth;
    float v = (src.y + tex->v) / tex->fheight;
    float u2 = (src.prom_x() + tex->u) / tex->fwidth;
    float v2 = (src.prom_y() + tex->v) / tex->fheight;

    if (flag & FX_BFLAG_FLIP_X)
        std::swap(u, u2);
    if ((flag & FX_BFLAG_FLIP_Y) || tex->__is_framebuffer)
        std::swap(v, v2);

    float x = dst.x, y = dst.y, w = dst.width, h = dst.height;

    *buf << x + w << y + h;
    __w_half(buf, vertex_color[2]);
    *buf << u2 << v;
    *buf << x + w << y;
    __w_half(buf, vertex_color[3]);
    *buf << u2 << v2;
    *buf << x << y;
    __w_half(buf, vertex_color[0]);
    *buf << u << v2;
    *buf << x << y + h;
    __w_half(buf, vertex_color[1]);
    *buf << u << v;
    buf->end_quad();
}

void brush::draw_texture(shared<texture> tex, const quad &dst, brush_flag flag)
{
    draw_texture(tex, dst, {0.0, 0.0, tex->width, tex->height});
}

void brush::draw_rect(const quad &dst)
{
    auto buf = lock_buffer();

    assert_mode(FX_COLORED_QUAD);

    float x = dst.x, y = dst.y, w = dst.width, h = dst.height;

    *buf << x + w << y + h;
    __w_half(buf, vertex_color[2]);
    *buf << x + w << y;
    __w_half(buf, vertex_color[3]);
    *buf << x << y;
    __w_half(buf, vertex_color[0]);
    *buf << x << y + h;
    __w_half(buf, vertex_color[1]);
    buf->end_quad();
}

void brush::draw_rect_outline(const quad &dst)
{
    float x = dst.x;
    float y = dst.y;
    float width = dst.width;
    float height = dst.height;

    draw_line({x, y}, {x + width, y});
    draw_line({x, y}, {x, y + height});
    draw_line({x + width, y}, {x + width, y + height});
    draw_line({x, y + height}, {x + width, y + height});
}

void brush::draw_triagle(const vec2 &p1, const vec2 &p2, const vec2 &p3)
{
    auto buf = lock_buffer();

    assert_mode(FX_COLORED_TRIANGLE);

    *buf << float(p1.x) << float(p1.y);
    __w_half(buf, vertex_color[0]);
    *buf << float(p2.x) << float(p2.y);
    __w_half(buf, vertex_color[1]);
    *buf << float(p3.x) << float(p3.y);
    __w_half(buf, vertex_color[2]);
    buf->new_vertex(3);
}

void brush::draw_line(const vec2 &p1, const vec2 &p2)
{
    auto buf = lock_buffer();

    assert_mode(FX_COLORED_LINE);

    *buf << float(p1.x) << float(p1.y);
    __w_half(buf, vertex_color[0]);
    *buf << float(p2.x) << float(p2.y);
    __w_half(buf, vertex_color[1]);
    buf->new_vertex(2);
}

void brush::draw_point(const vec2 &p)
{
    auto buf = lock_buffer();

    assert_mode(FX_COLORED_POINT);

    *buf << float(p.x) << float(p.y);
    __w_half(buf, vertex_color[0]);
    buf->new_vertex(1);
}

void brush::draw_oval(const quad &dst, int segs)
{
    if (segs <= 1)
        prtlog_throw(FATAL, "at least drawing an oval needs 2 segments.");

    float x = dst.center_x();
    float y = dst.center_y();
    float width = dst.width;
    float height = dst.height;

    float onerad = 2 * 3.1415926535 / segs;
    for (float i = 0; i < 2 * 3.1415926535; i += onerad)
    {
        float x1 = x + cosf(i) * width;
        float y1 = y + sinf(i) * height;
        float x2 = x + cosf(i + onerad) * width;
        float y2 = y + sinf(i + onerad) * height;
        draw_triagle({x, y}, {x1, y1}, {x2, y2});
    }
}

void brush::draw_oval_outline(const quad &dst, int segs)
{
    if (segs <= 1)
        prtlog_throw(FATAL, "at least drawing an oval needs 2 segments.");

    float x = dst.center_x();
    float y = dst.center_y();
    float width = dst.width;
    float height = dst.height;

    float onerad = 2 * 3.1415926535 / segs;
    for (float i = 0; i < 2 * 3.1415926535; i += onerad)
    {
        float x1 = x + cosf(i) * width;
        float y1 = y + sinf(i) * height;
        float x2 = x + cosf(i + onerad) * width;
        float y2 = y + sinf(i + onerad) * height;
        draw_line({x1, y1}, {x2, y2});
    }
}

void brush::clear(const color &col)
{
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void brush::viewport(const quad &quad)
{
    flush();
    glViewport(quad.x, quad.y, quad.width, quad.height);
}

void brush::scissor(const quad &quad)
{
    flush();
    glScissor(quad.x, quad.y, quad.width, quad.height);
}

void brush::scissor_end()
{
    flush();
    vec2 v = tk_get_size();
    glScissor(0, 0, v.x, v.y);
}

void brush::use(blend_mode mode)
{
    flush();
    if (mode == FX_NORMAL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else if (mode == FX_ADDITIVE_BLEND)
        glBlendFunc(GL_ONE, GL_ONE);
}

unique<brush> make_brush(shared<complex_buffer> buf)
{
    unique<brush> brush_ptr = std::make_unique<brush>();
    brush_ptr->buffer = buf;
    return brush_ptr;
}

} // namespace flux