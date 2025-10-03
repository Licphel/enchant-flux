#include <gfx/camera.h>
#include <gfx/gfx.h>

namespace flux::gfx
{

void camera::apply()
{
    double w = view.width / 2;
    double h = view.height / 2;

    projection_t.orthographic(-w, w, -h, h);
    translation_t.identity();

// since in flux GL the vertices kept pointing upwards, transformations are done here.
// explanation: why don't negate in shader - if do so, we have to translate the whole scene by view.h. that's terrible.
// and this leads to a new problem - the matrix passed to gpu differs from we actually use.
// so the solution is as follows - keep two versions of the transformation, rendering using #combined_out_t
// and projection using combined_t.
#ifdef FX_Y_IS_DOWN
    translation_t.translate(-center.x, -center.y);
    translation_t.rotate(rotation);
    translation_t.scale(scale.x, scale.y);
    combined_t.load(projection_t);
    combined_t.multiply(translation_t);
    inverted_t = combined_t.get_invert();

    translation_t.identity();
    translation_t.translate(-center.x, center.y);
    translation_t.rotate(rotation);
    translation_t.scale(scale.x, -scale.y);
    combined_out_t.load(projection_t);
    combined_out_t.multiply(translation_t);
#else
    translation_t.translate(-center.x, -center.y);
    translation_t.rotate(rotation);
    translation_t.scale(scale.x, scale.y);
    combined_t.load(projection_t);
    combined_t.multiply(translation_t);
    inverted_t = combined_t.get_invert();
    combined_out_t.load(combined_t);
#endif
}

void camera::set_to_static_center()
{
    center = vec2(view.width / 2.0, view.height / 2.0);
}

void camera::project(vec2 &v)
{
    if (viewport.width <= 0 || viewport.height <= 0)
    {
        // avoid NaN problem
        return;
    }
    combined_t.apply(v);
    v.x = viewport.width * (v.x + 1) / 2 + viewport.x;
    v.y = viewport.height * (v.y + 1) / 2 + viewport.y;
}

void camera::unproject(vec2 &v)
{
    if (viewport.width <= 0 || viewport.height <= 0)
    {
        // avoid NaN problem
        return;
    }
    v.x = 2.0 * (v.x - viewport.x) / viewport.width - 1.0;
    v.y = 2.0 * (v.y - viewport.y) / viewport.height - 1.0;
    inverted_t.apply(v);
}

double camera::project_x(double x)
{
    vec2 v = vec2(x, 0);
    project(v);
    return v.x;
}

double camera::project_y(double y)
{
    vec2 v = vec2(0, y);
    project(v);
    return v.y;
}

double camera::unproject_x(double x)
{
    vec2 v = vec2(x, 0);
    unproject(v);
    return v.x;
}

double camera::unproject_y(double y)
{
    vec2 v = vec2(0, y);
    unproject(v);
    return v.y;
}

static camera __cam_abs = {};
static camera __cam_gui = {};
static camera __cam_world = {};

camera &get_absolute_camera()
{
    vec2 size = tk_get_size();
    if (size.x == 0 || size.y == 0)
        return __cam_abs;
    __cam_abs.view = quad(0, 0, size.x, size.y);
    __cam_abs.set_to_static_center();
    __cam_abs.apply();
    __cam_abs.viewport = __cam_abs.view;
    return __cam_abs;
}

camera &get_gui_camera(bool only_int, double fixed_resolution)
{
    vec2 size = tk_get_size();
    if (size.x == 0 || size.y == 0)
        return __cam_gui;
    double factor = fixed_resolution;
    if (fixed_resolution <= 0)
    {
        factor = 0.5;
        while (size.x / (factor + 0.5) >= 800.0 && size.y / (factor + 0.5) >= 450.0)
            factor += 0.5;

        if (only_int && int(factor * 2) % 2 != 0 && factor - 0.5 > 0)
            factor -= 0.5;
    }
    __cam_gui.view = quad(0, 0, size.x / factor, size.y / factor);
    __cam_gui.set_to_static_center();
    __cam_gui.apply();
    __cam_gui.viewport = quad(0, 0, size.x, size.y);
    return __cam_gui;
}

camera &get_world_camera(vec2 center, double sight_horizontal)
{
    vec2 size = tk_get_size();
    if (size.x == 0 || size.y == 0)
        return __cam_world;

    double rt = 800.0 / 450.0;
    double fw, fh;

    if (size.x / size.y > rt)
    {
        fw = size.x;
        fh = size.x / rt;
    }
    else
    {
        fh = size.y;
        fw = size.y * rt;
    }

    double x0 = (size.x - fw) / 2, y0 = (size.y - fh) / 2;
    __cam_world.center = center;
    __cam_world.view = {0, 0, sight_horizontal, sight_horizontal / rt};
    __cam_world.apply();
    __cam_world.viewport = {x0, y0, fw, fh};
    return __cam_world;
}

} // namespace flux::gfx