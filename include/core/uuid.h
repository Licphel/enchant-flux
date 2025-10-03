#pragma once
#include <vector>
#include <core/def.h>
#include <core/rand.h>

namespace flux
{

struct uuid
{
    std::vector<byte> bytes = std::vector<byte>(16, (byte)0);
    size_t __hash;

    uuid() = default;

    bool operator==(const uuid &other) const;
    bool operator<(const uuid &other) const;
    operator std::string() const;
};

// returns a null uuid, all bytes are 0.
uuid uuid_null();
// generate a random uuid with current time and a random number from #get_grand.
// hopefully they won't collide.
uuid uuid_generate();

} // namespace flux

namespace std
{

template <> struct hash<flux::uuid>
{
    std::size_t operator()(const flux::uuid &id) const noexcept
    {
        return id.__hash;
    }
};

} // namespace std