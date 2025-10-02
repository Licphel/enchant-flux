#include <algorithm>
#include <chrono>
#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/gfx.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <glfw/glfw3.h>
#include <al/alc.h>
#include <al/al.h>
#include <core/log.h>
#include <thread>
#include <vector>
#include <gfx/mesh.h>

namespace flux::gfx
{

GLFWwindow *window;

std::vector<std::function<void(double delta)>> event_tick;
std::vector<std::function<void(brush *brush, double partial)>> event_render;
std::vector<std::function<void()>> event_dispose;
std::vector<std::function<void(int w, int h)>> event_resize;
std::vector<std::function<void(int button, int action, int mods)>> event_mouse_state;
std::vector<std::function<void(double x, double y)>> event_cursor_pos;
std::vector<std::function<void(double x, double y)>> event_mouse_scroll;
std::vector<std::function<void(int button, int scancode, int action, int mods)>> event_key_state;

long keydown[512];
long keydown_render[512];
char keymod[512];
char keyact[512];
double mcx, mcy;
double mscx, mscy;
double t_secs;
int lf_ticks, lf_render_ticks;
double lf_delta, lf_partial;
int rfps, rtps;
bool __cur_in_tick;
std::string __char_seq;

// global gfx usage
shared<mesh> direct_mesh = nullptr;

double tk_nanos()
{
    return glfwGetTime() * 1'000'000'000;
}

double tk_seconds()
{
    return t_secs;
}

int tk_ticks()
{
    return lf_ticks;
}

double tk_partial()
{
    return lf_partial;
}

double tk_delta()
{
    return lf_delta;
}

double tk_lerp(double old, double now)
{
    return old + (now - old) * lf_partial;
}

int tk_real_fps()
{
    return rfps;
}

int tk_real_tps()
{
    return rtps;
}

void tk_make_handle()
{
    if (!glfwInit())
        prtlog_throw(FX_FATAL, "glfw cannot initialize.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(128, 128, "", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        prtlog_throw(FX_FATAL, "glfw make window failed.");
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        prtlog_throw(FX_FATAL, "glew cannot initialize.");

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int nw, int nh) {
        for (auto e : event_resize)
            e(nw, nh);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int mods) {
        button += 480; /* **magic number** _TK_MOUSE_OFFSET */
        for (auto e : event_mouse_state)
            e(button, action, mods);
        keydown[button] = lf_ticks;
        keydown_render[button] = lf_render_ticks;
        keyact[button] = action;
        keymod[button] = mods;
    });
    glfwSetScrollCallback(window, [](GLFWwindow *, double x, double y) {
        for (auto e : event_mouse_scroll)
            e(x, y);
        mscx = x;
        mscy = y;
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) {
        for (auto e : event_cursor_pos)
            e(x, y);
        mcx = x;
#ifdef FX_Y_IS_DOWN
        mcy = y;
#else
        mcy = tk_get_size().y - y;
#endif
    });
    glfwSetKeyCallback(window, [](GLFWwindow *, int button, int scancode, int action, int mods) {
        for (auto e : event_key_state)
            e(button, scancode, action, mods);
        keydown[button] = lf_ticks;
        keydown_render[button] = lf_render_ticks;
        keyact[button] = action;
        keymod[button] = mods;
    });
    glfwSetCharCallback(window, [](GLFWwindow *, unsigned int codepoint) {
        general_char ch = (general_char)codepoint;
        __char_seq += ch;
    });

    // set opengl caps
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *txt, const void *) { printf("GL: %s\n", txt); },
        nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

void tk_title(const std::string &title)
{
    glfwSetWindowTitle(window, title.c_str());
}

void tk_size(vec2 size)
{
    glfwSetWindowSize(window, size.x, size.y);
    vec2 dsize = tk_get_device_size();
    tk_pos((dsize - size) / 2);
}

void tk_pos(vec2 pos)
{
    glfwSetWindowPos(window, pos.x, pos.y);
}

vec2 tk_get_device_size()
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vidm = glfwGetVideoMode(monitor);
    return vec2(vidm->width, vidm->height);
}

void tk_visible(bool visible)
{
    if (visible)
        glfwShowWindow(window);
    else
        glfwHideWindow(window);
}

void tk_maximize()
{
    glfwMaximizeWindow(window);
}

void tk_icon(shared<image> img)
{
    GLFWimage img_glfw;
    img_glfw.pixels = img->pixels;
    img_glfw.width = img->width;
    img_glfw.height = img->height;
    glfwSetWindowIcon(window, 1, &img_glfw);
}

void tk_end_make_handle()
{
    glfwShowWindow(window);
}

std::string tk_get_title()
{
    return glfwGetWindowTitle(window);
}

vec2 tk_get_size()
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    return vec2(w, h);
}

vec2 tk_get_pos()
{
    int x, y;
    glfwGetWindowPos(window, &x, &y);
    return vec2(x, y);
}

void tk_lifecycle(int fps, int tps, bool vsync)
{
    // init global gfx usage
    direct_mesh = make_mesh();
    direct_mesh->__is_direct = true;
    // end region
    glfwSwapInterval(vsync ? 1 : 0);

    const double DT_LOGIC_NS = 1'000'000'000.0 / tps;
    const double DT_RENDER_NS = (fps > 0) ? 1'000'000'000.0 / fps : 0.0;

    double current = tk_nanos();
    double logic_debt = 0.0;
    double last_render = current;

    rfps = fps;
    rtps = tps;
    lf_delta = DT_LOGIC_NS / 1'000'000'000.0;

    int tick_frm = 0, render_frm = 0;
    double last_calc = current;
    double last_stat = current;

    try
    {
        while (!tk_poll_events())
        {
            current = tk_nanos();

            logic_debt += (current - last_calc);
            last_calc = current;

            int max_catch = 4;
            while (logic_debt >= DT_LOGIC_NS && max_catch--)
            {
                __cur_in_tick = true;
                for (auto e : event_tick)
                    e(lf_delta);
                __cur_in_tick = false;
                lf_ticks++;
                t_secs += lf_delta;
                tick_frm++;
                logic_debt -= DT_LOGIC_NS;
            }

            if (fps <= 0 || current - last_render >= DT_RENDER_NS)
            {
                lf_partial = 1.0 - (logic_debt / DT_LOGIC_NS);
                lf_partial = lf_partial < 0.0 ? 0.0 : lf_partial > 1.0 ? 1.0 : lf_partial;

                auto brush = direct_mesh->brush_binded.get();
                for (auto e : event_render)
                    e(brush, lf_partial);
                lf_render_ticks++;
                brush->flush();
                tk_swap_buffers();

                render_frm++;
                last_render += DT_RENDER_NS;
                if (last_render < current - DT_RENDER_NS)
                    last_render = current;
            }

            if (current - last_stat > 500'000'000.0)
            {
                rtps = tick_frm * 2;
                rfps = render_frm * 2;
                tick_frm = render_frm = 0;
                last_stat = current;
            }
        }
    }
    catch (std::exception &e)
    {
        prtlog(FX_FATAL, "fatal error occurred: {}", e.what());
        // re-throw for debugging & stack trace.
        throw e;
    }

    for (auto e : event_dispose)
        e();
}

void tk_swap_buffers()
{
    glfwSwapBuffers(window);
}

bool tk_poll_events()
{
    glfwPollEvents();

    bool v = glfwWindowShouldClose(window);
    if (v)
        glfwTerminate();
    return v;
}

void tk_hook_event_tick(std::function<void(double delta)> callback)
{
    event_tick.push_back(callback);
}

void tk_hook_event_render(std::function<void(brush *brush, double partial)> callback)
{
    event_render.push_back(callback);
}

void tk_hook_event_dispose(std::function<void()> callback)
{
    event_dispose.push_back(callback);
}

void tk_hook_event_resize(std::function<void(int w, int h)> callback)
{
    event_resize.push_back(callback);
}

void tk_hook_mouse_state(std::function<void(int button, int action, int mods)> callback)
{
    event_mouse_state.push_back(callback);
}

void tk_hook_cursor_pos(std::function<void(double x, double y)> callback)
{
    event_cursor_pos.push_back(callback);
}

void tk_hook_key_state(std::function<void(int button, int scancode, int action, int mods)> callback)
{
    event_key_state.push_back(callback);
}

bool tk_key_held(int key, int mod)
{
    return keyact[key] != GLFW_RELEASE && (mod == MOD_ANY || keymod[key] & mod);
}

bool tk_key_press(int key, int mod)
{
    bool pressc =
        (__cur_in_tick && keydown[key] == lf_ticks) || (!__cur_in_tick && keydown_render[key] == lf_render_ticks);
    return keyact[key] == GLFW_PRESS && pressc && (mod == MOD_ANY || keymod[key] & mod);
}

vec2 tk_get_cursor()
{
    return vec2(mcx, mcy);
}

double tk_get_scroll()
{
    return std::abs(mscy);
}

int tk_get_scroll_towards()
{
    if (mscy > 10E-4)
        return SCROLL_UP;
    if (mscy < -10E-4)
        return SCROLL_DOWN;
    return SCROLL_NO;
}

std::string tk_consume_chars()
{
    std::string cpy = __char_seq;
    __char_seq.clear();
    return cpy;
}

} // namespace flux::gfx
