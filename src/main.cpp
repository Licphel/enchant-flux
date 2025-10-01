#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/toolkit.h>
#include <core/log.h>
#include <audio/aud.h>
#include <gfx/atlas.h>
#include <gfx/mesh.h>
#include <gfx/font.h>
#include <core/bin.h>
#include <core/buffer.h>
#include <core/bio.h>
#include <core/load.h>
#include <core/rand.h>
#include <core/uuid.h>

using namespace flux;
using namespace flux::gfx;

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

    tk_hook_event_tick([](double) {
        while (loader->progress < 1)
        {
            loader->next();
            prtlog(FX_INFO, "load: " + std::to_string(loader->progress));
        }
        tk_title(fmt::format("Enchant Flux | tps :{} fps :{}", tk_real_tps(), tk_real_fps()));
        if (tk_key_press(KEY_G))
            prtlog(FX_INFO, "HI!");
    });
    tk_hook_event_render([](brush *brush, double) {
        brush->clear({0, 0, 0, 1});
        brush->use(FX_NORMAL_BLEND);
        brush->use(get_world_camera({0, 0}, 250 + tk_ticks()));

        brush->draw_rect_outline({-10, -10, 5, 5});

        auto q = fnt->make_vtx(brush, "Font test", 0, 0, 1, 120);
        brush->draw_rect_outline(q);
        brush->cl_set(color(0, 1, 1, 1));
        brush->draw_line(vec2(-1000, 0), vec2(1000, 0));
        brush->draw_line(vec2(0, -1000), vec2(0, 1000));
        brush->cl_norm();

        brush->cl_set(color(1, 1, 1, 0.5));
        for (int i = 0; i < 10; i++)
            brush->draw_texture(tex1, quad(i * 40, i * 40, 50, 50));
        brush->cl_norm();

        if (tk_key_press(KEY_F))
            prtlog(FX_INFO, "HI!");
    });

    tk_lifecycle(0, 20, false);
    return 0;
}
