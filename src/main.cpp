#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/toolkit.h>
#include <kernel/common.h>
#include <audio/aud.h>
#include <gfx/atlas.h>
#include <gfx/mesh.h>

using namespace flux;

shared<texture> tex, tex1;
shared<mesh> msh;

int main()
{
    tk_make_handle();
    tk_title("Enchant: Flux CE");
    tk_size(vec2(800, 450));
    auto img = load_image(open_local("gfx/misc/icon.png"));
    auto img1 = load_image(open_local("gfx/misc/logo_flux.png"));
    shared<image_atlas> atlas = make_image_atlas(1024, 1024, 1);
    atlas->begin();
    atlas->accept(img);
    tex = atlas->accept(img1);
    tex1 = atlas->accept(img);
    atlas->end();
    tk_icon(img);
    tk_end_make_handle();
    msh = make_mesh();

    tk_hook_event_tick([](double delta) {
        tk_title(fmt::format("Enchant: Flux CE t:{} f:{}", tk_real_tps(), tk_real_fps()));
        if (tk_key_press(KEY_G))
            prtlog(INFO, "HI!");
    });
    tk_hook_event_render([](brush *brush, double partial) {
        brush->clear({1, 0, 0, 1});
        brush->use(FX_NORMAL_BLEND);
        brush->use(get_world_camera({tk_ticks() / 50.0, 0}, 100 + tk_ticks() * 2.5));
        brush->draw_oval_outline({-100, -100, 50, 50});

        if (msh->buffer->vertex_count == 0)
        {
            auto brush_1 = msh->retry();
            for (int i = 0; i < 512; i++)
            {
                brush_1->draw_texture(tex, {i / 2.0, 0, 93, 31});
                brush_1->draw_texture(tex1, {i / 2.0, 40, 93, 31});
            }
            msh->record();
        }
        else
        {
            msh->draw(brush);
        }

        if (tk_key_press(KEY_F))
            prtlog(INFO, "HI!");
    });

    au_make_device();
    au_end_make_device();

    auto trk = au_load_track(open_local("audio/night_theme.wav"));
    auto clp = au_make_clip(trk);
    clp->operate(FX_CLIP_PLAY);

    tk_lifecycle(0, 20, false);
    return 0;
}
