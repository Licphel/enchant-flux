#include <core/buffer.h>

namespace flux
{

bool __check_is_sysle()
{
    constexpr uint32_t test_value = 0x01020304;
    return reinterpret_cast<const byte *>(&test_value)[0] == 0x04;
}

byte_buf::byte_buf() = default;

byte_buf::byte_buf(size_t initial_size) : __data(initial_size)
{
}

byte_buf::byte_buf(const std::vector<byte> &vec)
{
    __data = vec;
    __wpos = vec.size();
}

size_t byte_buf::size() const
{
    return __wpos;
}

size_t byte_buf::capacity() const
{
    return __data.capacity();
}

size_t byte_buf::free_bytes() const
{
    return __data.capacity() - __wpos;
}

size_t byte_buf::remaining() const
{
    return __data.size() - __wpos;
}

size_t byte_buf::readable_bytes() const
{
    return __wpos - __rpos;
}

bool byte_buf::is_empty() const
{
    return __wpos == 0;
}

void byte_buf::clear()
{
    __rpos = 0;
    __wpos = 0;
}

void byte_buf::reserve(size_t size)
{
    __data.reserve(size);
}

void byte_buf::resize(size_t size)
{
    __data.resize(size);
    if (__wpos > size)
        __wpos = size;
    if (__rpos > size)
        __rpos = size;
}

void byte_buf::ensure_capacity(size_t needed)
{
    if (remaining() < needed)
        __data.resize(__data.size() + std::max(needed, __data.size() * 2));
}

void byte_buf::write_bytes(const void *src, size_t len)
{
    ensure_capacity(len);
    std::memcpy(__data.data() + __wpos, src, len);
    __wpos += len;
}

void byte_buf::write_byte_buf(const byte_buf &buf)
{
    write<size_t>(buf.size());
    write_bytes(buf.__data.data(), buf.size());
}

void byte_buf::write_string(const std::string &str)
{
    write<size_t>(str.size());
    write_bytes(str.data(), str.size());
}

void byte_buf::read_bytes(void *dst, size_t len)
{
    std::memcpy(dst, __data.data() + __rpos, len);
    __rpos += len;
}

byte_buf byte_buf::read_byte_buf()
{
    size_t size = read<size_t>();
    byte_buf buf = byte_buf(size);
    read_bytes(buf.__data.data(), size);
    return buf;
}

std::string byte_buf::read_string()
{
    size_t len = read<size_t>();
    std::string str(reinterpret_cast<const char *>(__data.data() + __rpos), len);
    __rpos += len;
    return str;
}

void byte_buf::skip(size_t len)
{
    __rpos += len;
}

void byte_buf::set_read_pos(size_t pos)
{
    __rpos = pos;
}

void byte_buf::set_write_pos(size_t pos)
{
    if (pos > __data.size())
        __data.resize(pos);
    __wpos = pos;
    if (__rpos > __wpos)
        __rpos = __wpos;
}

size_t byte_buf::read_pos() const
{
    return __rpos;
}

size_t byte_buf::write_pos() const
{
    return __wpos;
}

std::vector<byte> byte_buf::to_vector() const
{
    return std::vector<byte>(__data.begin(), __data.begin() + __wpos);
}

std::vector<byte> byte_buf::read_advance(int len)
{
    size_t readable = readable_bytes();
    if (len > readable)
        len = readable;
    auto vec = std::vector<byte>(__data.begin() + __rpos, __data.begin() + __rpos + len);
    __rpos += len;
    return vec;
}

void byte_buf::rewind()
{
    __rpos = 0;
}

void byte_buf::compact()
{
    if (__rpos > 0)
    {
        size_t remaining = __wpos - __rpos;
        if (remaining > 0)
            std::memmove(__data.data(), __data.data() + __rpos, remaining);
        __wpos = remaining;
        __rpos = 0;
    }
}

} // namespace flux
