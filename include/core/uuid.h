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

    bool operator==(const uuid &other) const
    {
        if(__hash != other.__hash)
            return false;
        return bytes == other.bytes;
    }

    bool operator<(const uuid &other) const
    {
        return bytes < other.bytes;
    }

    operator std::string() const
    {
        std::string result;
        result.reserve(32);

        const char hex_chars[] = "0123456789abcdef";

        for (byte byte : bytes)
        {
            result += hex_chars[(byte >> 4) & 0x0F];
            result += hex_chars[byte & 0x0F];
        }

        return result;
    }
};

uuid uuid_null();
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