#include <kernel/common.h>
#include <vector>

namespace flux
{

std::string __get_header(log_type type)
{
    std::string header;
    switch (type)
    {
    case DEBUG:
        header = "[DEBUG] ";
        break;
    case INFO:
        header = "[INFO] ";
        break;
    case WARN:
        header = "[WARN] ";
        break;
    case FATAL:
        header = "[FATAL] ";
        break;
    };
    return header;
}

}