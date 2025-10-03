#pragma once
#include <gfx/brush.h>
#include <gfx/image.h>
#include <audio/au.h>

namespace flux::gfx
{

struct gui_component
{
    quad region;
    vec2 cursor_pos;
    int __index_in_gui;

    virtual ~gui_component() = default;
    virtual void render(brush *brush) = 0;
    virtual void tick() = 0;
};

struct gui : std::enable_shared_from_this<gui>
{
    static std::vector<shared<gui>> CURRENTS;
    static void render_currents(brush *brush);
    static void tick_currents();

    std::vector<shared<gui_component>> __components;
    std::function<void(shared<gui> g)> on_closed;
    std::function<void(shared<gui> g)> on_displayed;

    void join(shared<gui_component> comp);
    void remove(shared<gui_component> comp);
    void clear();
    void display();
    void close();

    virtual void render(brush *brush);
    virtual void tick();
};

template <class T, class... Args> shared<T> make_gui_component(Args &&...args)
{
    static_assert(std::is_base_of<gui_component, T>::value, "T must be a subclass of gui_component.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T, class... Args> shared<T> make_gui(Args &&...args)
{
    static_assert(std::is_base_of<gui, T>::value, "T must be a subclass of gui.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

struct gui_button : gui_component
{
    enum button_state
    {
        IDLE,
        PRESSED,
        HOVERING
    };

    std::function<void(brush *brush, button_state state)> on_render;
    std::function<void()> on_click;
    std::function<void()> on_right_click;
    bool enable_switching = false;
    button_state curstate = IDLE;

    void render(brush *brush) override;
    void tick() override;
};

} // namespace flux::gfx