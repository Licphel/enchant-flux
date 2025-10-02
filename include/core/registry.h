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
    registry<T> *reg;
    int idx;

    operator T()
    {
        return *reg[idx];
    }
};

template <typename T> struct registry
{
    bool __index_only = false;
    std::map<res_id, T> map;
    std::stack<std::function<void()>> __delayed;
    int __idx_next = 0;

    ref<T> make(const res_id &id, const T &obj)
    {
        map[id] = obj;
        int idx = __idx_next++;
        __delayed.push([this, idx, id]() {
            map[id].reg_index = idx;
            if(!__index_only)
                map[id].reg_id = id;
        });
         return {this, idx};
    }

    void work()
    {
        while(!__delayed.empty())
        {
            __delayed.top()();
            __delayed.pop();
        }
    }

    registry<T>& index_only()
    {
        __index_only = true;
        return *this;
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
