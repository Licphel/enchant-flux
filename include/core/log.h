#pragma once
#include <iostream>
#include <fmt/core.h>
#include <string>
#include <stdexcept>

namespace flux
{

enum log_type
{
    FX_DEBUG = 0,
    FX_INFO = 1,
    FX_WARN = 2,
    FX_FATAL = 3
};

std::string __get_header(log_type type);

template <typename... Args> void prtlog(log_type type, const std::string &fmt, Args &&...args)
{
    std::string formatted = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << __get_header(type) << formatted << '\n';
}

template <typename... Args> [[noreturn]] void prtlog_throw(log_type type, const std::string &fmt, Args &&...args)
{
    std::string formatted = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << __get_header(type) << formatted << '\n';
    throw std::runtime_error(formatted);
}

} // namespace flux
