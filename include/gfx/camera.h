#pragma once
#include <gfx/color.h>
#include <math/mat.h>
#include <math/quad.h>
#include <math/vec.h>
#include <memory>

namespace flux
{

enum transform_space
{
    FX_TRANSFORM_ABS,
    FX_TRANSFORM_WORLD,
    FX_TRANSFORM_GUI
};

struct camera
{
    vec2 center = vec2(0, 0);
    transform projection_t = {};
    transform translation_t = {};
    transform combined_t = {};
    transform inverted_t = {};
    double rotation = 0.0;
    vec2 scale = vec2(1, 1);
    quad view = {};
    quad viewport = {};

    void apply();
    void set_to_static_center();
    void project(vec2 &v);
    void unproject(vec2 &v);
    double project_x(double x);
    double project_y(double y);
    double unproject_x(double x);
    double unproject_y(double y);
};

camera &get_absolute_camera();
camera &get_gui_camera(bool only_int = false, double fixed_resolution = -1);
camera &get_world_camera(vec2 center, double sight_horizontal);

} // namespace flux