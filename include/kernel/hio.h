#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <kernel/def.h>

namespace fs = std::filesystem;

namespace flux
{

struct hpath
{
    std::string absolute;
    /* unstable */ fs::path __npath;

    hpath(std::string &name) : absolute(name), __npath(fs::path(name))
    {
        __check();
    }

    hpath(const std::string &name) : absolute(name), __npath(fs::path(name))
    {
        __check();
    }

    void __check()
    {
        for (size_t pos = 0; (pos = absolute.find('\\', pos)) != std::string::npos;)
            absolute.replace(pos, 1, "/");
    }

    hpath operator/(const std::string &name) const
    {
        bool append = absolute[absolute.length() - 1] != '/';
        return hpath(append ? absolute + '/' + name : absolute + name);
    }
};

enum hpath_type
{
    FX_DIR,
    FX_FILE,
    FX_UNKNOWN
};

hpath open(const std::string &name);
hpath open_local(const std::string &name);
hpath parent(const hpath &path);
void del(const hpath &path);
void rename(const hpath &path, const std::string &name);
bool exists(const hpath &path);
void mkdirs(const hpath &path);
hpath_type judge(const hpath &path);
std::vector<hpath> sub_dirs(const hpath &path);
std::vector<hpath> sub_files(const hpath &path);
std::vector<hpath> recurse_files(const hpath &path);
hpath execution_path();

std::vector<byte> read_bytes(const hpath &path);
void write_bytes(const hpath &path, const std::vector<byte> &data);
std::string read_str(const hpath &path);
void write_str(const hpath &path, const std::string &text);

} // namespace flux