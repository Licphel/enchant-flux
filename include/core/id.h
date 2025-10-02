#pragma once
#include <string>
#include <functional>

namespace flux
{

struct res_scope
{
    std::string key;

    res_scope();
    res_scope(const std::string &key);
};

struct res_id
{
    res_scope scope;
    std::string key;
    std::string concat;
    size_t __hash;

    res_id();
    res_id(const std::string &cat);
    res_id(const res_scope &sc, const std::string &k);
    res_id(const char ch_arr[]);

    operator std::string() const;
    bool operator ==(const res_id& other) const;
    bool operator <(const res_id& other) const;
};

} // namespace flux

namespace std
{

template <> struct hash<flux::res_id>
{
    std::size_t operator()(const flux::res_id &id) const noexcept
    {
        return id.__hash;
    }
};

} // namespace std
