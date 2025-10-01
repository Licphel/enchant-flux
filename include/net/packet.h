#pragma once
#include <core/buffer.h>
#include <core/hio.h>
#include <core/uuid.h>

namespace flux::net
{

struct packet
{
    uuid sender = {};
    bool invalid = false;

    virtual ~packet() = default;
    virtual void read(const std::vector<uint8_t> &buf) = 0;
    virtual void write(std::vector<uint8_t> &buf) const = 0;
    virtual void perform() = 0;

    static std::vector<byte> pack(const packet &p, int pid);
    static unique<packet> unpack(std::vector<byte> in, int pid);
};

} // namespace flux
