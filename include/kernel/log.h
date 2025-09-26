#pragma once
#include <iostream>
#include <fmt/core.h>
#include <string>
#include <stdexcept>

namespace flux
{

enum log_type
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    FATAL = 3
};

std::string __get_header(log_type type);

template <typename... T> void prtlog(log_type type, const std::string &text, T &&...args)
{
    std::string formatted = fmt::format(text, std::forward<T>(args)...);
    std::cout << __get_header(type) << formatted << std::endl;
}

template <typename... T> [[noreturn]] void prtlog_throw(log_type type, const std::string &text, T &&...args)
{
    std::string formatted = fmt::format(text, std::forward<T>(args)...);
    std::cout << __get_header(type) << formatted << std::endl;
    throw std::runtime_error(formatted); // here throws the exception, when a fatal error occurs.
}

} // namespace flux
