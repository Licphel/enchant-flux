#include <gfx/gui.h>

namespace flux::gfx
{

std::vector<shared<gui>> gui::CURRENTS;

void gui::render_currents(brush *brush)
{
    for (auto &g : CURRENTS)
        g->render(brush);
}

void gui::tick_currents()
{
    for (auto &g : CURRENTS)
        g->tick();
}

void gui::join(shared<gui_component> comp)
{
    comp->__index_in_gui = (int)__components.size();
    __components.push_back(comp);
}

void gui::remove(shared<gui_component> comp)
{
    int idx = comp->__index_in_gui;
    __components.erase(__components.begin() + idx);
    for (int i = idx; i < (int)__components.size(); i++)
        __components[i]->__index_in_gui = i;
    comp->__index_in_gui = -1;
}

void gui::clear()
{
    __components.clear();
}

void gui::display()
{
    CURRENTS.push_back(shared_from_this());
    if (on_displayed)
        on_displayed(shared_from_this());
}

void gui::close()
{
    CURRENTS.erase(
        std::remove_if(CURRENTS.begin(), CURRENTS.end(), [this](const shared<gui> &g) { return g.get() == this; }),
        CURRENTS.end());
    if (on_closed)
        on_closed(shared_from_this());
}

void gui::render(brush *brush)
{
    auto cam = get_gui_camera();
    auto old_cam = brush->camera_binded;
    brush->use(cam);
    vec2 cursor_projected = tk_get_cursor();
    // transform cursor to world coordinates
    cam.unproject(cursor_projected);

    for (auto &c : __components)
    {
        c->cursor_pos = cursor_projected;
        c->render(brush);
    }

    brush->use(old_cam);
}

void gui::tick()
{
    for (auto &c : __components)
        c->tick();
}

// ---------------- gui_button ----------------

void gui_button::render(brush *brush)
{
    on_render(brush, curstate);
}

void gui_button::tick()
{
    bool hovering = quad::contain(region, cursor_pos);
    bool ld = tk_key_press(FX_MOUSE_BUTTON_LEFT);
    bool rd = tk_key_press(FX_MOUSE_BUTTON_RIGHT);

    if (hovering)
    {
        if (ld && on_click)
            on_click();
        if (rd && on_right_click)
            on_right_click();
    }

    if (enable_switching)
    {
        if (hovering && (ld || rd))
            curstate = (curstate == PRESSED) ? (hovering ? HOVERING : IDLE) : PRESSED;
    }
    else
    {
        curstate = IDLE;
        if (hovering)
        {
            curstate = HOVERING;
            if (ld)
                curstate = PRESSED;
            if (rd)
                curstate = PRESSED;
        }
    }
}

} // namespace flux::gfx