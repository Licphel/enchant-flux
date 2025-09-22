#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/toolkit.h>
#include <kernel/common.h>
#include <audio/aud.h>
#include <gfx/atlas.h>
#include <gfx/mesh.h>
#include <gfx/font.h>

using namespace flux;

shared<texture> tex, tex1;
shared<mesh> msh;
shared<font> fnt;

int main()
{
    tk_make_handle();
    tk_title("Enchant Flux");
    tk_size(vec2(800, 450));
    auto img = load_image(open_local("gfx/misc/icon.png"));
    auto img1 = load_image(open_local("gfx/misc/test.png"));
    shared<atlas> atlas = make_atlas(1024, 1024);
    atlas->begin();
    atlas->accept(img);
    tex = atlas->accept(img1);
    tex1 = atlas->accept(img);
    atlas->end();
    tk_icon(img);
    tk_end_make_handle();
    msh = make_mesh();
    fnt = ft_make_font(open_local("gfx/font/fusion_pixel.ttf"), 12, 12);

    tk_hook_event_tick([](double delta) {
        tk_title(fmt::format("Enchant Flux | tps :{} fps :{}", tk_real_tps(), tk_real_fps()));
        if (tk_key_press(KEY_G))
            prtlog(INFO, "HI!");
    });
    tk_hook_event_render([](brush *brush, double partial) {
        brush->clear({0, 0, 0, 1});
        brush->use(FX_NORMAL_BLEND);
        brush->use(get_world_camera({0, 0}, 250 + tk_ticks()));

        brush->draw_rect_outline({-10, -10, 5, 5});

        auto q = fnt->make_vtx(brush, "Font test", 0, 0, 1, 120);
        brush->draw_rect_outline(q);
        brush->cl_set({0, 1, 1, 1});
        brush->draw_line({-1000, 0}, {1000, 0});
        brush->draw_line({0, -1000}, {0, 1000});
        brush->cl_norm();

        if (tk_key_press(KEY_F))
            prtlog(INFO, "HI!");
    });

    au_make_device();
    au_end_make_device();

    // auto trk = au_load_track(open_local("audio/night_theme.wav"));
    // auto clp = au_make_clip(trk);
    // clp->operate(FX_CLIP_PLAY);

    tk_lifecycle(0, 20, false);
    return 0;
}
