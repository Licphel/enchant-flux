#pragma once
#include <core/hio.h>
#include <core/id.h>
#include <map>
#include <stack>
#include <any>
#include <functional>

namespace flux
{

std::unordered_map<res_id, std::any> *__get_resource_map();

// get a loaded resource by its id.
// if you are unsure if the resource is loaded, use #make_res_ref instead.
template <typename T> T make_res(const res_id &id)
{
    const auto m = __get_resource_map();
    auto it = m->find(id);
    bool cf = it != m->end() && it->second.has_value();
    return cf ? std::any_cast<T>(it->second) : std::decay_t<T>{};
}

template <typename T> struct aref
{
    res_id id;

    bool is_done() const
    {
        const auto m = __get_resource_map();
        return m->find(id) != m->end();
    }

    operator T() const
    {
        return make_res<T>(id);
    }
};

using proc_strategy = std::function<void(const hio_path &path, const res_id &id)>;

struct asset_loader
{
    res_scope scope;
    hio_path root;
    double progress;
    int __done_tcount;
    int __total_tcount;
    std::map<std::string, proc_strategy> process_strategy_map;
    std::stack<std::function<void()>> tasks;
    std::vector<shared<asset_loader>> subloaders;
    std::function<void()> event_on_start;
    std::function<void()> event_on_end;
    bool __start_called;
    bool __end_called;

    void scan(const hio_path &path_root);
    void add_sub(shared<asset_loader> subloader);
    // run a task in the queue.
    // you may need to check the #progress to see if all tasks are done.
    void next();
};

// when you are unsure if the resource is loaded, use this to get a reference to it.
template <typename T> aref<T> make_res_ref(const res_id &id)
{
    return {id};
}

shared<asset_loader> make_loader(const res_scope &scope, const hio_path &root);

enum asset_loader_equip
{
    FX_LOAD_PNG_AS_TEXTURE,
    FX_LOAD_PNG_AS_IMAGE,
    FX_LOAD_TXT,
    FX_LOAD_FONT,
    FX_LOAD_SHADER,
    FX_LOAD_WAVE,
    FX_LOAD_SCRIPT
};

// add a built-in loader behavior to the loader.
void make_loader_equipment(shared<asset_loader> loader, asset_loader_equip equipment);

} // namespace flux
