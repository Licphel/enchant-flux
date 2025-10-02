#pragma once
#include <core/buffer.h>
#include <core/hio.h>
#include <core/uuid.h>
#include <core/registry.h>
#include <functional>
#include <core/registry.h>

namespace flux::net
{

struct packet_context
{
    virtual void hold_alive(const uuid &id) = 0;
};

struct packet : std::enable_shared_from_this<packet>
{
    uuid sender;
    bool invalid = false;

    virtual ~packet() = default;
    virtual void read(byte_buf &buf) = 0;
    virtual void write(byte_buf &buf) const = 0;
    virtual void perform(packet_context *ctx) = 0;

    // packet procotol:
    // unzipped int: LENGTH
    // zipped int: PID
    // zipped int: DATA

    static std::vector<byte> pack(shared<packet> p);
    static shared<packet> unpack(byte_buf &buf, int len);

    void send_to_server();
    void send_to_remote(const uuid &rid);
    void send_to_remotes(const std::vector<uuid> &rids);
    void send_to_remotes();
};

struct packet_2s_heartbeat : packet
{
    packet_2s_heartbeat() = default;

    void read(byte_buf &) override
    {
    }

    void write(byte_buf &) const override
    {
    }

    void perform(packet_context *ctx) override
    {
        ctx->hold_alive(sender);
    }
};

struct packet_dummy : packet
{
    std::string str;

    packet_dummy() = default;
    packet_dummy(const std::string &str) : str(str)
    {
    }

    void read(byte_buf &buf) override
    {
        str = buf.read_string();
    }

    void write(byte_buf &buf) const override
    {
        buf.write_string(str);
    }

    void perform(packet_context *) override
    {
        prtlog(FX_DEBUG, str);
    }
};

template <typename T, typename... Args> shared<packet> make_packet(Args &&...args)
{
    static_assert(std::is_base_of_v<packet, T>, "make a non-packet object.");
    return std::make_unique<T>(std::forward<Args>(args)...);
}

int __pid_counter();
std::map<int, std::function<shared<packet>()>> &__pmap();
std::map<size_t, int> &__pmap_rev();

template <typename T> void register_packet()
{
    const auto &tid = typeid(T);
    int pid = __pid_counter();
    __pmap()[pid] = []() { return make_packet<T>(); };
    __pmap_rev()[tid.hash_code()] = pid;
}

} // namespace flux::net
