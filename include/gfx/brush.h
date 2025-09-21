#pragma once
#include <cstring>
#include <gfx/camera.h>
#include <gfx/color.h>
#include <gfx/toolkit.h>
#include <gfx/buf.h>
#include <math/mat.h>
#include <math/vec.h>
#include <stack>

namespace flux
{

enum brush_shape
{
    FX_BRUSH_RECT,
    FX_BRUSH_OVAL,
    FX_BRUSH_POINT,
    FX_BRUSH_LINE,
    FX_BRUSH_TRIANGLE
};

enum brush_flag
{
    FX_BFLAG_NO = 1LL << 0,
    FX_BFLAG_FLIP_X = 1LL << 1,
    FX_BFLAG_FLIP_Y = 1LL << 2
};

enum blend_mode
{
    FX_NORMAL_BLEND,
    FX_ADDITIVE_BLEND
};

struct graph_state
{
    graph_mode mode = FX_TEXTURED_QUAD;
    shared<texture> texture = nullptr;
    shared<shader_program> program = nullptr;
    void (*callback_uniform)(shared<shader_program> program) = nullptr;
    void (*callback_buffer_append)(unique<complex_buffer> buf) = nullptr;
};

struct brush
{
    color vertex_color[4]{};
    std::stack<transform> transform_stack;
    camera camera_binded;
    graph_state m_state;
    shared<shader_program> __default_colored;
    shared<shader_program> __default_textured;
    weak<complex_buffer> buffer;
    bool __is_in_mesh = false;

    brush();

    shared<complex_buffer> lock_buffer();
    graph_state &current_state();
    void cl_norm();
    void cl_set(const color &col);
    void cl_mrg(const color &col);
    void cl_mrg(double v);
    void ts_push();
    void ts_pop();
    void ts_load(const transform &t);
    void ts_trs(const vec2 &v);
    transform get_combined_transform();

    void flush();
    void assert_mode(graph_mode mode);
    void assert_texture(shared<texture> tex);
    void use(const camera &cam);
    void use(shared<shader_program> program);
    void use(const graph_state& sts);

    void draw_texture(shared<texture> tex, const quad &dst, const quad &src, brush_flag flag = FX_BFLAG_NO);
    void draw_texture(shared<texture> tex, const quad &dst, brush_flag flag = FX_BFLAG_NO);
    void draw_rect(const quad &dst);
    void draw_rect_outline(const quad &dst);
    void draw_triagle(const vec2 &p1, const vec2 &p2, const vec2 &p3);
    void draw_line(const vec2 &p1, const vec2 &p2);
    void draw_point(const vec2 &p);
    void draw_oval(const quad &dst, int segs = 16);
    void draw_oval_outline(const quad &dst, int segs = 16);

    void clear(const color &col);
    void viewport(const quad &quad);
    void scissor(const quad &quad);
    void scissor_end();
    void use(blend_mode mode);
};

unique<brush> make_brush(shared<complex_buffer> buf);

} // namespace flux