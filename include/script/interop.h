#pragma once
#include <string>
#include <kernel/def.h>
#include <stdarg.h>

#define MODULIZED

namespace flux
{

// not ensured to be right.
// users should not use these.
enum __dvdbg_type
{
    __IP_NULL,
    __IP_INT,
    __IP_DOUBLE,
    __IP_STR,
    __IP_BOOL,
    __IP_MODULE,
    __IP_UNDEF,
    __IP_FUNC,
    __IP_UNKNOWN
};

struct dynvalue
{
    struct _impl;
    shared<_impl> __p;
    __dvdbg_type __type;

    dynvalue(shared<_impl> __p);
    ~dynvalue();

    static dynvalue vint(int v);
    static dynvalue vdouble(double v);
    static dynvalue vstr(const std::string &v);
    static dynvalue vbool(bool v);
    static dynvalue veval(const std::string &name, const std::string &code);
    static dynvalue vgval(const std::string &key);

    dynvalue vsub(const std::string &key);
    dynvalue vcall(int argc, ...);
};

void ip_init();

} // namespace flux
