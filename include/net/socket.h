#pragma once
#include <net/packet.h>
#include <core/uuid.h>

namespace flux::net
{

enum class connection_type
{
    // the server is integrated, in the same process with the remote.
    integrated_server,
    // the server is connected to the remote via lan.
    lan_server,
    // the server is connected to the remote via ip:port.
    address_server
};

// a packet-socket.
// a socket can be either a server or a remote socket.
struct socket : packet_context
{
    socket();
    ~socket();

    // remote starts
    void connect(connection_type type, const std::string &host = "", uint16_t port = 0);
    void send_to_server(shared<packet> pkt);
    void disconnect();
    // discover lan server, and connect to it.
    void discover();

    // server starts
    void start(uint16_t port = 0);
    void stop();
    void send_to_remote(const uuid &remote_id, shared<packet> pkt);
    void send_to_remotes(shared<packet> pkt);

    // process packets. this should be called in the main thread.
    void tick();
    void hold_alive(const uuid& id);

    struct _impl;
    unique<_impl> __p;
};

socket* get_gsocket_server();
socket* get_gsocket_remote();

} // namespace flux::net
