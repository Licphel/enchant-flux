#include <core/id.h>

namespace flux
{

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

} // namespace flux
