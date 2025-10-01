#pragma once
#include <core/def.h>
#include <string>
#include <vector>
#include <core/log.h>
#include <core/hio.h>
#include <map>
#include <any>
#include <core/buffer.h>
#include <variant>

namespace flux
{

enum __bincvt_typem
{
    __BINCVT_BYTE,
    __BINCVT_SHORT,
    __BINCVT_INT,
    __BINCVT_LONG,
    __BINCVT_FLOAT,
    __BINCVT_DOUBLE,
    __BINCVT_STRING_C,
    __BINCVT_BOOL,
    __BINCVT_MAP,
    __BINCVT_ARRAY,
    __BINCVT_BUF,

    __BINCVT_EOF = 255
};

struct binary_map;
struct binary_array;

struct binary_value
{
    __bincvt_typem type;
    std::any __anyv;

    template <typename T> static binary_value make(const T &v)
    {
        using decay_t = std::decay_t<T>;

        if constexpr (std::is_same_v<byte, decay_t>)
            return {__BINCVT_BYTE, std::any(v)};
        else if constexpr (std::is_same_v<short, decay_t>)
            return {__BINCVT_SHORT, std::any(v)};
        else if constexpr (std::is_same_v<int, decay_t>)
            return {__BINCVT_INT, std::any(v)};
        else if constexpr (std::is_same_v<long, decay_t>)
            return {__BINCVT_LONG, std::any(v)};
        else if constexpr (std::is_same_v<float, decay_t>)
            return {__BINCVT_FLOAT, std::any(v)};
        else if constexpr (std::is_same_v<double, decay_t>)
            return {__BINCVT_DOUBLE, std::any(v)};
        else if constexpr (std::is_constructible_v<std::string, decay_t>)
            return {__BINCVT_STRING_C, std::any(std::string(v))};
        else if constexpr (std::is_same_v<bool, decay_t>)
            return {__BINCVT_BOOL, std::any(v)};
        else if constexpr (std::is_same_v<binary_map, decay_t>)
            return {__BINCVT_MAP, std::any(v)};
        else if constexpr (std::is_same_v<binary_array, decay_t>)
            return {__BINCVT_ARRAY, std::any(v)};
        else if constexpr (std::is_same_v<byte_buf, decay_t>)
            return {__BINCVT_BUF, std::any(v)};

        prtlog_throw(FX_FATAL, "unsupported type.");
    }

    template <typename T> T cast() const
    {
        switch (type)
        {
        case __BINCVT_BYTE:
            if constexpr (std::is_convertible_v<byte, T>)
                return static_cast<T>(std::any_cast<byte>(__anyv));
            else
                break;
        case __BINCVT_SHORT:
            if constexpr (std::is_convertible_v<short, T>)
                return static_cast<T>(std::any_cast<short>(__anyv));
            else
                break;
        case __BINCVT_INT:
            if constexpr (std::is_convertible_v<int, T>)
                return static_cast<T>(std::any_cast<int>(__anyv));
            else
                break;
        case __BINCVT_LONG:
            if constexpr (std::is_convertible_v<long, T>)
                return static_cast<T>(std::any_cast<long>(__anyv));
            else
                break;
        case __BINCVT_FLOAT:
            if constexpr (std::is_convertible_v<float, T>)
                return static_cast<T>(std::any_cast<float>(__anyv));
            else
                break;
        case __BINCVT_DOUBLE:
            if constexpr (std::is_convertible_v<double, T>)
                return static_cast<T>(std::any_cast<double>(__anyv));
            else
                break;
        case __BINCVT_STRING_C:
            if constexpr (std::is_convertible_v<std::string, T>)
                return std::any_cast<std::string>(__anyv);
            else
                break;
        case __BINCVT_BOOL:
            if constexpr (std::is_convertible_v<bool, T>)
                return static_cast<T>(std::any_cast<bool>(__anyv));
            else
                break;
        case __BINCVT_MAP:
            if constexpr (std::is_convertible_v<binary_map, T>)
                return static_cast<T>(std::any_cast<binary_map>(__anyv));
            else
                break;
        case __BINCVT_ARRAY:
            if constexpr (std::is_convertible_v<binary_array, T>)
                return static_cast<T>(std::any_cast<binary_array>(__anyv));
            else
                break;
        case __BINCVT_BUF:
            if constexpr (std::is_convertible_v<byte_buf, T>)
                return static_cast<T>(std::any_cast<byte_buf>(__anyv));
            else
                break;
        default:
            prtlog_throw(FX_FATAL, "not convertible.");
        }
        prtlog_throw(FX_FATAL, "not convertible.");
    }
};

struct binary_map
{
    struct __proxy
    {
        binary_map &src;
        std::string key;

        template <typename T> __proxy &operator=(T &&value)
        {
            src.set(key, std::forward<T>(value));
            return *this;
        }

        template <typename T> operator T() const
        {
            return src.get<T>(key);
        }
    };

    std::map<std::string, binary_value> data;

    size_t size() const
    {
        return data.size();
    }

    template <typename T> T get(const std::string &key, const T &def = T()) const
    {
        auto it = data.find(key);
        if (it == data.end())
            return def;
        return it->second.cast<T>();
    }

    bool has(const std::string &key)
    {
        return data.find(key) != data.end();
    }

    template <typename T> void set(const std::string &key, const T &val)
    {
        data[key] = binary_value::make(val);
    }

    __proxy operator[](const std::string &key)
    {
        return {*this, key};
    }
};

struct binary_array
{
    struct __proxy
    {
        binary_array &src;
        int i;

        template <typename T> __proxy &operator=(T &&value)
        {
            src.set(i, std::forward<T>(value));
            return *this;
        }

        template <typename T> operator T() const
        {
            return src.get<T>(i);
        }
    };

    std::vector<binary_value> data;

    size_t size() const
    {
        return data.size();
    }

    template <typename T> T get(int i, const T &def = T()) const
    {
        if (i < 0 || i >= data.size())
            return def;
        return data[i].cast<T>();
    }

    template <typename T> void set(int i, const T &val)
    {
        data[i] = binary_value::make(val);
    }

    template <typename T> void push(const T &val)
    {
        data.push_back(binary_value::make(val));
    }

    __proxy operator[](int i)
    {
        return {*this, i};
    }
};

} // namespace flux