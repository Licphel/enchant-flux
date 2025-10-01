#pragma once
#include <vector>
#include <core/def.h>
#include <core/rand.h>

namespace flux
{

struct uuid
{
    std::vector<byte> bytes = std::vector<byte>(16, (byte)0);

    bool operator==(const uuid &other)
    {
        return bytes == other.bytes;
    }

    operator std::string()
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

uuid uuid_generate();

} // namespace flux