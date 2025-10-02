#include <core/id.h>

namespace flux
{

res_scope::res_scope() = default;

res_scope::res_scope(const std::string &key) : key(key)
{
}

res_id::res_id() = default;

res_id::res_id(const std::string &cat)
{
    concat = cat;
    auto pos = cat.find_last_of(':');
    key = cat.substr(pos + 1);
    scope = cat.substr(0, pos);
    __hash = std::hash<std::string>{}(concat);
}

res_id::res_id(const res_scope &sc, const std::string &k)
{
    scope = sc;
    key = k;
    concat = scope.key + ":" + key;
    __hash = std::hash<std::string>{}(concat);
}

res_id::res_id(const char ch_arr[]) : res_id(std::string(ch_arr))
{
}

res_id::operator std::string() const
{
    return concat;
}

bool res_id::operator==(const res_id &other) const
{
    if (other.__hash != __hash)
        return false;
    return other.concat == concat;
}

bool res_id::operator<(const res_id &other) const
{
    return other.concat < concat;
}

} // namespace flux
