#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/gfx.h>
#include <core/log.h>
#include <audio/au.h>
#include <gfx/atlas.h>
#include <gfx/mesh.h>
#include <gfx/font.h>
#include <core/bin.h>
#include <core/buffer.h>
#include <core/bio.h>
#include <core/load.h>
#include <core/rand.h>
#include <core/uuid.h>
#include <net/packet.h>
#include <net/socket.h>
#include <gfx/gui.h>

using namespace flux;
using namespace flux::gfx;
using namespace flux::net;
using namespace flux::au;

#define NET_TEST

#ifndef NET_TEST
shared<texture> tex, tex1;
shared<mesh> msh;
shared<font> fnt;
shared<asset_loader> loader;

int main()
{

    loader = make_loader({"enchant"}, hio_open_local(""));
    make_loader_equipment(loader, FX_LOAD_PNG_AS_TEXTURE);
    loader->scan(hio_open_local(""));

    /*
    std::string strscr = hio_read_str(hio_open_local("script/main.qk"));

    Interpreter *itp = new Interpreter();
    set_library_functions(itp);
    itp->addNative(
        "function prtlog(str)", [](Variable *c, void *data) { prtlog(FX_INFO, c->getParameter("str")->getString()); },
    0); itp->execute(strscr);
    */
    std::string strscr = hio_read_str(hio_open_local("script/main.qk"));
    binary_map map = bio_read_langd(hio_open_local("script/main.qk"));

    tk_make_handle();
    tk_title("Enchant Flux");
    tk_size(vec2(800, 450));
    auto img = load_image(hio_open_local("gfx/misc/icon.png"));
    auto img1 = load_image(hio_open_local("gfx/misc/test.png"));
    shared<atlas> atlas = make_atlas(1024, 1024);
    atlas->begin();
    atlas->accept(img);
    tex = atlas->accept(img1);
    tex1 = atlas->accept(img);
    atlas->end();
    tk_icon(img);
    tk_end_make_handle();
    msh = make_mesh();
    fnt = load_font(hio_open_local("gfx/font/fusion_pixel.ttf"), 12, 12);

    tk_hook_event_tick([]() {
        while (loader->progress < 1)
        {
            loader->next();
            prtlog(FX_INFO, "load: " + std::to_string(loader->progress));
        }
        tk_title(fmt::format("Enchant Flux | tps :{} fps :{}", tk_real_tps(), tk_real_fps()));
        if (tk_key_press(FX_KEY_G))
            prtlog(FX_INFO, "HI!");
    });
    tk_hook_event_render([](brush *brush) {
        brush->clear({0, 0, 0, 1});
        brush->use(FX_NORMAL_BLEND);
        brush->use(get_world_camera({0, 0}, 250 + tk_ticks()));

        brush->draw_rect_outline({-10, -10, 5, 5});

        auto q =
            fnt->make_vtx(brush, "Font test", 0, 0, 1, 120);
        brush->cl_norm();
        brush->draw_rect_outline(q);
        brush->cl_set(color(0, 1, 1, 1));
        brush->draw_line(vec2(-1000, 0), vec2(1000, 0));
        brush->draw_line(vec2(0, -1000), vec2(0, 1000));
        brush->cl_norm();

        brush->cl_set(color(1, 1, 1, 0.5));
        for (int i = 0; i < 10; i++)
            brush->draw_texture(tex1, quad(i * 40, i * 40, 50, 50));
        brush->cl_norm();

        if (tk_key_press(FX_KEY_F))
            prtlog(FX_INFO, "HI!");
    });

    tk_lifecycle(0, 20, false);
    return 0;
}
#else

int i;
socket *sockc = get_gsocket_remote();
socket *socks = get_gsocket_server();
shared<gui> g;
shared<gui_button> b;

int main()
{
    register_packet<packet_2s_heartbeat>();
    register_packet<packet_dummy>();

    g = make_gui<gui>();
    b = make_gui_component<gui_button>();
    b->region = quad::corner(100, 100, 200, 50);
    b->on_render = [](brush *brush, gui_button::button_state state) {
        if (state == gui_button::IDLE)
            brush->cl_set(color(1, 1, 1, 1));
        else if (state == gui_button::HOVERING)
            brush->cl_set(color(0.8, 0.8, 1, 1));
        else if (state == gui_button::PRESSED)
            brush->cl_set(color(0.6, 0.6, 1, 1));
        brush->draw_rect(b->region);
        brush->cl_norm();
    };
    b->on_click = []() { prtlog(FX_INFO, "clicked!"); };
    g->join(b);
    g->display();
    socks->start(8080);
    sockc->connect(connection_type::lan_server, "127.0.0.1", 8080);

    tk_hook_event_tick([]() { gui::tick_currents(); });

    tk_hook_event_render([](brush *brush) {
        brush->clear({0, 0, 0, 1});
        brush->use(FX_NORMAL_BLEND);
        gui::render_currents(brush);
    });

    tk_make_handle();
    tk_title("Enchant Flux");
    tk_size(vec2(800, 450));
    tk_end_make_handle();
    tk_lifecycle(60, 20, false);

    tk_make_device();
    tk_set_device_option(FX_AUDIO_ROLLOFF, 2.0);
    tk_set_device_option(FX_AUDIO_REFERENCE_DIST, 8.0);
    tk_set_device_option(FX_AUDIO_MAX_DIST, 42.0);
    tk_set_device_option(FX_AUDIO_LISTENER, vec3(0, 0, 0));
    tk_end_make_device();
}
#endif
