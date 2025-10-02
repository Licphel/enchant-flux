#include <net/packet.h>
#include <core/hio.h>
#include <net/socket.h>

namespace flux::net
{

std::vector<byte> packet::pack(shared<packet> p)
{
    auto n = typeid(*p).hash_code();
    auto it = __pmap_rev().find(n);

    if (it == __pmap_rev().end())
        prtlog_throw(FX_FATAL, "unregistered packet.");

    int pid = it->second;
    byte_buf uncmped_buf;
    uncmped_buf.write<int>(pid);
    p->write(uncmped_buf);

    auto cmped_buf = hio_compress(uncmped_buf.to_vector(), FX_COMP_OPTIMAL);
    uncmped_buf.set_write_pos(0);
    uncmped_buf.write<int>(cmped_buf.size());
    uncmped_buf.write_bytes(cmped_buf.data(), cmped_buf.size());
    int size = static_cast<int>(uncmped_buf.size());

    if (size > 32767)
        prtlog_throw(FX_FATAL, "too large packet with {} bytes!", size);

    return uncmped_buf.to_vector();
}

shared<packet> packet::unpack(byte_buf &buffer, int len)
{
    auto dcmped_buf = hio_decompress(buffer.read_advance(len));
    byte_buf buf = byte_buf(std::move(dcmped_buf));

    buf.set_write_pos(dcmped_buf.size());
    int pid = buf.read<int>();

    auto it = __pmap().find(pid);

    if (it == __pmap().end())
        prtlog_throw(FX_FATAL, "unregistered packet.");

    shared<packet> p = it->second();
    p->read(buf);
    return p;
}

void packet::send_to_server()
{
    get_gsocket()->send_to_server(shared_from_this());
}

void packet::send_to_remote(const uuid &rid)
{
    get_gsocket()->send_to_remote(rid, shared_from_this());
}

void packet::send_to_remotes(const std::vector<uuid> &rids)
{
    auto ptr = shared_from_this();
    auto skt = get_gsocket();
    for (auto &r : rids)
        skt->send_to_remote(r, ptr);
}

void packet::send_to_remotes()
{
    get_gsocket()->send_to_remotes(shared_from_this());
}

static int __pid_counter_v;
static std::map<int, std::function<shared<packet>()>> __pmap_v;
static std::map<size_t, int> __pmap_rev_v;

int __pid_counter()
{
    return __pid_counter_v++;
}

std::map<int, std::function<shared<packet>()>> &__pmap()
{
    return __pmap_v;
}

std::map<size_t, int> &__pmap_rev()
{
    return __pmap_rev_v;
}

} // namespace flux::net
