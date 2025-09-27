#include <kernel/log.h>
#include <vector>

namespace flux
{

std::string __get_header(log_type type)
{
    std::string header;
    switch (type)
    {
    case FX_DEBUG:
        header = "[DEBUG] ";
        break;
    case FX_INFO:
        header = "[INFO] ";
        break;
    case FX_WARN:
        header = "[WARN] ";
        break;
    case FX_FATAL:
        header = "[FATAL] ";
        break;
    };
    return header;
}

}