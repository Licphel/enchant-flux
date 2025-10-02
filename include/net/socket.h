#pragma once
#include <net/packet.h>
#include <core/uuid.h>

namespace flux::net
{

enum class connection_type
{
    integrated_server,
    lan_server,
    address_server
};

struct socket : packet_context
{
    socket();
    ~socket();

    // remote methods
    void connect(connection_type type, const std::string &host = "", uint16_t port = 0);
    void send_to_server(shared<packet> pkt);
    void disconnect();
    void discover();

    // server methods
    void start(uint16_t port = 0);
    void stop();
    void send_to_remote(const uuid &remote_id, shared<packet> pkt);
    void send_to_remotes(shared<packet> pkt);

    void tick();
    void hold_alive(const uuid& id);

    struct _impl;
    unique<_impl> __p;
};

socket* get_gsocket();

} // namespace flux::net
