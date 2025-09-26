#pragma once
#include <kernel/def.h>
#include <string>
#include <vector>
#include <kernel/log.h>
#include <kernel/hio.h>
#include <map>
#include <any>

namespace flux
{

struct bino
{
    struct __proxy
    {
        bino &src;
        std::string key;

        template <typename T> __proxy &operator=(T &&value)
        {
            src.set(key, std::forward<T>(value));
            return *this;
        }

        template <class T> operator T() const
        {
            return src.get<T>(key);
        }
    };

    std::map<std::string, std::any> data;

    template <typename T> T get(const std::string &key, const T &def = T())
    {
        auto it = data.find(key);
        if (it == data.end())
            return def;

        try
        {
            return std::any_cast<T>(it->second);
        }
        catch (const std::bad_any_cast &)
        {
            prtlog_throw(FATAL, "bino: cannot cast key '{}' to requested type", key);
        }
        return def;
    }

    template <typename T> void set(const std::string &key, const T &val)
    {
        data[key] = std::any(val);
    }

    __proxy operator[](const std::string &key)
    {
        return {*this, key};
    }
};

struct binar
{
    struct __proxy
    {
        binar &src;
        int i;

        template <typename T> __proxy &operator=(T &&value)
        {
            src.set(i, std::forward<T>(value));
            return *this;
        }

        template <class T> operator T() const
        {
            return src.get<T>(i);
        }
    };

    std::vector<std::any> data;

    template <typename T> T get(int i, const T &def = T())
    {
        if (i < 0 || i >= data.size())
            return def;
        try
        {
            return std::any_cast<T>(data[i]);
        }
        catch (const std::bad_any_cast &)
        {
            prtlog_throw(FATAL, "binar: cannot cast index '{}' to requested type", i);
        }
        return def;
    }

    template <typename T> void set(int i, const T &val)
    {
        data[i] = std::any(val);
    }

    template <typename T> void push(const T &val)
    {
        data.push_back(std::any(val));
    }

    __proxy operator[](int i)
    {
        return {*this, i};
    }
};

} // namespace flux