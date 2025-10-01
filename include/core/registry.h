#pragma once
#include <map>
#include <core/id.h>
#include <core/def.h>
#include <functional>
#include <stack>

namespace flux
{

template <typename T> struct registry;

template <typename T> struct ref
{
    registry *reg;
    int idx;

    operator T()
    {
        return *reg[idx];
    }
};

template <typename T> struct registry
{
    std::map<res_id, T> map;
    std::stack<std::function<void()>> __delayed;
    int __idx_next = 0;

    ref<T> make(const res_id &id, const T &obj)
    {
        map[id] = obj;
        int idx = __idx_next++;
        __delayed.push([map, idx, id]() {
            map[id].reg_index = idx;
            map[id].reg_id = id;
        });
         return {this, idx};
    }

    T operator[](int idx)
    {
        return map[idx];
    }

    T operator[](const res_id &id)
    {
        return map[id];
    }
};

} // namespace flux
