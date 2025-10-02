#include <core/load.h>
#include <gfx/image.h>
#include <gfx/font.h>
#include <audio/au.h>

using namespace flux::gfx;
using namespace flux::au;

namespace flux
{

std::unordered_map<res_id, std::any> __resource_map;

std::unordered_map<res_id, std::any> *__get_resource_map()
{
    return &__resource_map;
}

void asset_loader::scan(const hio_path &path_root)
{
    for (const hio_path &path : hio_recurse_files(path_root))
    {
        if (hio_judge(path) == FX_FILE)
        {
            res_id id = res_id(scope, path - root);
            std::string fmt = path.file_format();

            if (process_strategy_map.find(fmt) != process_strategy_map.end())
            {
                proc_strategy sttg = process_strategy_map[fmt];
                tasks.push([sttg, path, id]() { sttg(path, id); });
                __total_tcount++;
            }
        }
    }
}

void asset_loader::add_sub(shared<asset_loader> subloader)
{
    subloaders.push_back(subloader);
    __total_tcount += subloader->__total_tcount;
}

void asset_loader::next()
{
    if (__end_called)
        return;

    if (!__start_called && event_on_start)
    {
        event_on_start();
        __start_called = true;
    }

    if (!tasks.empty())
    {
        auto fn = tasks.top();
        tasks.pop();
        fn();
        __done_tcount++;
        progress = double(__done_tcount) / double(__total_tcount);
    }
    else
    {
        progress = 1;

        bool can_find = false;
        for (auto sub : subloaders)
        {
            if (sub->progress < 1)
            {
                sub->next();
                can_find = true;
                break;
            }
        }
        if (!can_find)
        {
            if (!__end_called && event_on_end)
            {
                event_on_end();
                __end_called = true;
            }
        }
    }
}

shared<asset_loader> make_loader(const res_scope &scope, const hio_path &root)
{
    shared<asset_loader> lptr = std::make_shared<asset_loader>();
    lptr->scope = scope;
    lptr->root = root;
    return lptr;
}

void make_loader_equipment(shared<asset_loader> loader, asset_loader_equip equipment)
{
    switch (equipment)
    {
    case FX_LOAD_PNG_AS_TEXTURE:
        loader->process_strategy_map[".png"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(make_texture(load_image(path)));
        };
        break;
    case FX_LOAD_PNG_AS_IMAGE:
        loader->process_strategy_map[".png"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(load_image(path));
        };
        break;
    case FX_LOAD_TXT:
        loader->process_strategy_map[".txt"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(hio_read_str(path));
        };
        break;
    case FX_LOAD_WAVE:
        loader->process_strategy_map[".wav"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(load_track(path));
        };
        break;
    case FX_LOAD_FONT:
        // hard encoded warn: check later
        loader->process_strategy_map[".ttf"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(load_font(path, 12, 12));
        };
        loader->process_strategy_map[".otf"] = [](const hio_path &path, const res_id &id) {
            __resource_map[id] = std::any(load_font(path, 12, 12));
        };
        break;
    case FX_LOAD_SCRIPT:
    case FX_LOAD_SHADER:
        break;
    }
}

} // namespace flux
