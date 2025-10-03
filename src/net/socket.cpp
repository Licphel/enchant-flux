#include <net/socket.h>
#include <boost/asio.hpp>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <memory>
#include <fmt/format.h>
#include <queue>
#include <gfx/gfx.h>

namespace flux::net
{

static size_t NET_BUF_SIZE = 1024 * 1024;
static double NET_TIME_OUT = 5.0;

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

template <typename T> class __blocking_queue
{
  private:
    size_t capacity;
    std::queue<T> que;
    mutable std::mutex mtx;
    std::condition_variable cv_empty;
    std::condition_variable cv_full;

  public:
    explicit __blocking_queue(size_t cap = 0) : capacity(cap)
    {
    }

    void push(T item)
    {
        std::unique_lock<std::mutex> lk(mtx);
        if (capacity > 0)
            cv_full.wait(lk, [this] { return que.size() < capacity; });
        que.push(std::move(item));
        cv_empty.notify_one();
    }

    T take()
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv_empty.wait(lk, [this] { return !que.empty(); });
        T val = std::move(que.front());
        que.pop();
        cv_full.notify_one();
        return val;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lk(mtx);
        return que.size();
    }
};

struct socket::_impl
{
    asio::io_context ioc;
    std::thread worker;
    std::thread ioc_worker;
    std::mutex mtx;

    tcp::socket client_sock{ioc};
    byte_buf rcvbuf = byte_buf(NET_BUF_SIZE);

    tcp::acceptor acceptor{ioc};
    udp::socket broadcaster{ioc};
    std::thread broadcast_thread;
    uint16_t broadcast_port = 15000;

    struct channel
    {
        tcp::socket sock;
        byte_buf rcvbuf = byte_buf(NET_BUF_SIZE);
        __blocking_queue<shared<packet>> snd_packets;
        uuid id;
        std::thread worker;
        bool is_term = false;
        double last_beat;

        channel() = delete;
        channel(asio::io_context &ioc, uuid uid) : sock(ioc), id(uid)
        {
            worker = std::thread([this] { __write(); });
        }

        void __write()
        {
            auto pkt = snd_packets.take();
            auto buf = packet::pack(pkt);
            asio::async_write(sock, asio::buffer(buf), [this](std::error_code ec, size_t) {
                if (ec)
                    prtlog(FX_WARN, "fail to write async: {}", ec.message());
                if (is_term)
                    return;
                asio::post(sock.get_executor(), [this] { __write(); });
            });
        }
    };

    std::unordered_map<uuid, shared<channel>> channels;
    bool is_server = false;
    bool is_remote = false;
    std::atomic_bool is_term = false;
    std::vector<shared<packet>> rcv_packets;
    __blocking_queue<shared<packet>> snd_packets;
    double last_sec_event;

    _impl() : ioc(), client_sock(ioc), acceptor(ioc), broadcaster(ioc)
    {
    }

    void remote_connect(const std::string &host, uint16_t port)
    {
        is_remote = true;
        tcp::resolver resolver(ioc);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        asio::async_connect(client_sock, endpoints, [this, host, port](std::error_code ec, tcp::endpoint) {
            if (!ec)
            {
                prtlog(FX_INFO, "[remote] successfully connected to {}:{}", host, port);
                remote_read();
            }
            else
                prtlog_throw(FX_WARN, "[remote] cannot connect to {}:{}, cause: {}", host, port, ec.message());
        });

        worker = std::thread([this] {
            while (!is_term)
            {
                auto pkt = snd_packets.take();
                auto buf = packet::pack(pkt);
                asio::async_write(client_sock, asio::buffer(buf), [](std::error_code ec, size_t) {
                    if (ec)
                        prtlog(FX_WARN, "fail to write async: {}", ec.message());
                });
            }
        });
        ioc_worker = std::thread([this] { ioc.run(); });
    }

    void remote_disconnect()
    {
        ioc.stop();
        if (ioc_worker.joinable())
            ioc_worker.join();

        client_sock.close();
        is_remote = false;
        is_term = true;

        prtlog(FX_INFO, "[remote] remote disconnected.");
    }

    void __read_to_queue(int byte_read, byte_buf &buf, const uuid &id)
    {
        if (byte_read == 0)
        {
            if (is_remote)
                remote_disconnect();
            if (is_server)
                server_stop();
            return;
        }

        buf.set_write_pos(buf.write_pos() + byte_read);

        while (buf.readable_bytes() >= 4)
        {
            int rp = buf.read_pos();
            int len = buf.read<int>();

            if (buf.readable_bytes() < (size_t)len)
            {
                buf.set_read_pos(rp);

                if (buf.capacity() - buf.size() <= (size_t)len + 4)
                    buf.compact();
                break;
            }

            shared<packet> p = packet::unpack(buf, len);
            p->sender = id;

            if (buf.readable_bytes() <= 0)
                buf.clear();
            else if (buf.read_pos() >= buf.capacity() / 2)
                buf.compact();

            // lock is necessary.
            std::lock_guard<std::mutex> lk(mtx);
            rcv_packets.push_back(p);
        }
    }

    void remote_send(shared<packet> pkt)
    {
        snd_packets.push(pkt);
    }

    void remote_read()
    {
        client_sock.async_read_some(asio::buffer(rcvbuf.__data.data() + rcvbuf.write_pos(), rcvbuf.free_bytes()),
                                    [this](std::error_code ec, size_t n) {
                                        if (ec)
                                        {
                                            prtlog(FX_WARN, "[remote] connection is denied.");
                                            return;
                                        }
                                        __read_to_queue(n, rcvbuf, uuid_null());
                                        remote_read();
                                        if (is_term)
                                            return;
                                    });
    }

    void start_broadcast(uint16_t game_port)
    {
        broadcast_port = 15000;
        broadcaster.open(udp::v4());
        broadcaster.set_option(udp::socket::broadcast(true));
        broadcast_thread = std::thread([this, game_port]() {
            udp::endpoint broadcast_ep(asio::ip::make_address("255.255.255.255"), broadcast_port);
            while (!is_term)
            {
                std::string msg = "enchant server " + std::to_string(game_port);
                broadcaster.send_to(asio::buffer(msg), broadcast_ep);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    void stop_broadcast()
    {
        if (broadcast_thread.joinable())
        {
            broadcaster.close();
            broadcast_thread.join();
        }
    }

    void server_start(uint16_t port)
    {
        prtlog(FX_INFO, "[server] server is opened at port {}", port);

        is_server = true;

        tcp::endpoint ep(tcp::v4(), port);
        acceptor.open(ep.protocol());
        acceptor.set_option(tcp::acceptor::reuse_address(true));
        acceptor.bind(ep);
        acceptor.listen();
        server_accept();

        start_broadcast(port);

        ioc_worker = std::thread([this] { ioc.run(); });
    }

    void server_stop()
    {
        ioc.stop();
        if (ioc_worker.joinable())
            ioc_worker.join();

        prtlog(FX_INFO, "[server] server is stopped.");

        is_server = false;
        is_term = true;

        acceptor.close();
        stop_broadcast();
        for (auto &[id, r] : channels)
            r->sock.close();
        channels.clear();
    }

    void server_accept()
    {
        auto remote = std::make_shared<channel>(ioc, uuid_generate());

        acceptor.async_accept(remote->sock, [this, remote](std::error_code ec) {
            if (!ec)
            {
                prtlog(FX_INFO, "[server] server has connected remote {}", (std::string)remote.get()->id);
                server_read(remote);
                channels[remote->id] = remote;
            }
            if (is_term)
                return;
            server_accept();
        });
    }

    void server_read(shared<channel> r)
    {
        r->sock.async_read_some(asio::buffer(r->rcvbuf.__data.data() + r->rcvbuf.write_pos(), r->rcvbuf.free_bytes()),
                                [this, r](std::error_code ec, size_t n) {
                                    if (ec)
                                    {
                                        channels.erase(r->id);
                                        r->is_term = true;
                                        prtlog(FX_INFO, "[server] connection lost: {}", (std::string)r->id);
                                        return;
                                    }
                                    if (is_term)
                                        return;
                                    __read_to_queue(n, r->rcvbuf, r->id);
                                    server_read(r);
                                });
    }

    void server_send(const uuid &rid, shared<packet> pkt)
    {
        auto it = channels.find(rid);
        if (it == channels.end())
            return;
        it->second->snd_packets.push(pkt);
    }

    void server_send_every(shared<packet> pkt)
    {
        for (auto &[id, r] : channels)
            r->snd_packets.push(pkt);
    }

    void tick(socket *sk)
    {
        double now = gfx::tk_seconds();

        if (now - last_sec_event > 1.0)
        {
            last_sec_event = now;

            if (is_server)
            {
                auto it = channels.begin();
                while (it != channels.end())
                {
                    auto &c = *it;
                    if (now - c.second->last_beat > NET_TIME_OUT)
                    {
                        prtlog(FX_INFO, "remote channel {} timeout.", (std::string)c.second->id);
                        it = channels.erase(it);
                    }
                    else
                        ++it;
                }
            }
            if (is_remote)
                remote_send(make_packet<packet_2s_heartbeat>());
        }

        // lock is necessary.
        std::vector<shared<packet>> tmp;

        std::lock_guard<std::mutex> lk(mtx);
        tmp.swap(rcv_packets);

        for (auto &e : tmp)
            e->perform(sk);
    }

    void hold_alive(const uuid &rid)
    {
        auto it = channels.find(rid);
        if (it == channels.end())
            return;
        it->second->last_beat = gfx::tk_seconds();
    }
};

socket::socket() : __p(std::make_unique<_impl>())
{
}

socket::~socket()
{
    stop();
}

void socket::discover()
{
    udp::socket udp_sock(__p->ioc);
    udp_sock.open(udp::v4());
    udp_sock.bind(udp::endpoint(asio::ip::address_v4::any(), 15000));

    std::string addrs;
    uint16_t ports = 0;

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

    while (std::chrono::steady_clock::now() < deadline)
    {
        std::array<char, 256> recv_buf;
        udp::endpoint sender_ep;
        std::error_code ec;
        std::size_t len = udp_sock.receive_from(asio::buffer(recv_buf), sender_ep);

        if (!ec && len > 0)
        {
            std::string msg(recv_buf.data(), len);
            const std::string prefix = "enchant server ";
            if (msg.rfind(prefix, 0) == 0)
            {
                addrs = sender_ep.address().to_string();
                ports = static_cast<uint16_t>(std::stoi(msg.substr(prefix.size())));
                break;
            }
        }
    }
    udp_sock.close();

    if (!addrs.empty() && ports != 0)
    {
        prtlog(FX_INFO, "[remote] found lan server {}:{}, trying to connect now.", addrs, ports);
        __p->remote_connect(addrs, ports);
    }
    else
        prtlog(FX_WARN, "[remote] cannot find lan server nearby.");
}

void socket::connect(connection_type type, const std::string &host, uint16_t port)
{
    if (type == connection_type::integrated_server)
        __p->remote_connect("127.0.0.1", port ? port : 8080);
    else if (type == connection_type::lan_server)
        discover();
    else if (type == connection_type::address_server)
        __p->remote_connect(host, port);
}

void socket::send_to_server(shared<packet> pkt)
{
    __p->remote_send(pkt);
}

void socket::disconnect()
{
    __p->remote_disconnect();
}

void socket::start(uint16_t port)
{
    __p->server_start(port ? port : 8080);
}

void socket::stop()
{
    __p->server_stop();
}

void socket::send_to_remote(const uuid &rid, shared<packet> pkt)
{
    __p->server_send(rid, pkt);
}

void socket::send_to_remotes(shared<packet> pkt)
{
    __p->server_send_every(pkt);
}

void socket::tick()
{
    __p->tick(this);
}

void socket::hold_alive(const uuid &id)
{
    __p->hold_alive(id);
}

static socket __gsocket_s;
static socket __gsocket_c;

socket *get_gsocket_server()
{
    return &__gsocket_s;
}

socket *get_gsocket_remote()
{
    return &__gsocket_c;
}

} // namespace flux::net