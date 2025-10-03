#pragma once
#include <gfx/color.h>
#include <core/math.h>
#include <core/math.h>
#include <core/math.h>
#include <memory>

namespace flux::gfx
{

struct camera
{
    vec2 center = vec2(0, 0);
    transform projection_t = {};
    transform translation_t = {};
    transform combined_t = {};
    transform inverted_t = {};
    transform combined_out_t = {};
    double rotation = 0.0;
    vec2 scale = vec2(1, 1);
    quad view = {};
    quad viewport = {};

    void apply();
    // to let the camera look at center of the view.
    void set_to_static_center();
    // project world coordinates to screen coordinates.
    void project(vec2 &v);
    // project screen coordinates to world coordinates.
    void unproject(vec2 &v);
    double project_x(double x);
    double project_y(double y);
    double unproject_x(double x);
    double unproject_y(double y);
};

// returns a camera that maps 1:1 to the screen pixels.
camera &get_absolute_camera();
// returns a fixed-size camera whose view is [800, 450] (16:9).
// this keeps the layout consistent on different screen sizes.
// #only_int makes the scaling factor an integer, useful for pixel aligning.
// #fixed_resolution, if positive, forbids auto-scaling.
camera &get_gui_camera(bool only_int = false, double fixed_resolution = -1);
// returns a camera looking at a world position, with a given horizontal sight range.
// vertical sight range is unsure, because the ratio varies from device to device.
camera &get_world_camera(vec2 center, double sight_horizontal);

} // namespace flux