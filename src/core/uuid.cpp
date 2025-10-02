#include <core/uuid.h>
#include <chrono>
#include <random>

namespace flux
{

uuid uuid_null()
{
    return uuid();
}

uuid uuid_generate()
{
    uuid u;

    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    for (int i = 0; i < 8; i++)
        u.bytes[i] = (ns >> (56 - i * 8)) & 0xFF;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dis(0, 255);

    for (int i = 8; i < 16; i++)
        u.bytes[i] = dis(gen);

    u.__hash = std::hash<std::string_view>{}(std::string_view(reinterpret_cast<const char *>(u.bytes.data()), u.bytes.size()));

    return u;
}

} // namespace flux
