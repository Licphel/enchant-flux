#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <core/def.h>
#include <core/log.h>

namespace flux
{

// check if the system is little-endian.
bool __check_is_sysle();

// general byte buffer used in serialization, networking, etc.
// endian-aware.
struct byte_buf
{
    std::vector<byte> __data;
    size_t __rpos = 0;
    size_t __wpos = 0;
    bool __l_endian = __check_is_sysle();

    template <typename T> T swap_endian(T value) const
    {
        if constexpr (sizeof(T) == 1)
            return value;
        else
        {
            union {
                T value;
                byte bytes[sizeof(T)];
            } src, dst;

            src.value = value;
            for (size_t i = 0; i < sizeof(T); ++i)
                dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];
            return dst.value;
        }
    }

    template <typename T> T to_native_endian(T value) const
    {
        if (__l_endian)
            return value;
        return swap_endian(value);
    }

    byte_buf();
    byte_buf(size_t initial_size);
    byte_buf(const std::vector<byte> &vec);

    size_t size() const;
    size_t capacity() const;
    size_t free_bytes() const;
    size_t remaining() const;
    size_t readable_bytes() const;
    bool is_empty() const;
    void clear();
    void reserve(size_t size);
    void resize(size_t size);
    void ensure_capacity(size_t needed);
    void ensure_readable(size_t needed) const;

    template <typename T> typename std::enable_if<std::is_arithmetic<T>::value, void>::type write(T value)
    {
        ensure_capacity(sizeof(T));
        T network_value = to_native_endian(value);
        std::memcpy(__data.data() + __wpos, &network_value, sizeof(T));
        __wpos += sizeof(T);
    }

    void write_bytes(const void *src, size_t len);
    void write_byte_buf(const byte_buf &buf);
    void write_string(const std::string &str);

    template <typename T> typename std::enable_if<std::is_arithmetic<T>::value, T>::type read()
    {
        ensure_readable(sizeof(T));
        T value;
        std::memcpy(&value, __data.data() + __rpos, sizeof(T));
        __rpos += sizeof(T);
        return to_native_endian(value);
    }

    void read_bytes(void *dst, size_t len);
    byte_buf read_byte_buf();
    std::string read_string();

    template <typename T> T peek() const
    {
        ensure_readable(sizeof(T));
        T value;
        std::memcpy(&value, __data.data() + __rpos, sizeof(T));
        return to_native_endian(value);
    }

    void skip(size_t len);
    void set_read_pos(size_t pos);
    void set_write_pos(size_t pos);
    size_t read_pos() const;
    size_t write_pos() const;
    // copy the data before write position to a vector.
    std::vector<byte> to_vector() const;
    // read from current read position and advance it by len.
    std::vector<byte> read_advance(int len);
    // rewind so that we can read the data from the very beginning.
    void rewind();
    // remove the read data and move the unread data to the front.
    void compact();
};

} // namespace flux
