#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <core/def.h>

namespace fs = std::filesystem;

namespace flux
{

struct hio_path
{
    std::string absolute;
    /* unstable */ fs::path __npath;

    hio_path() = default;

    hio_path(const std::string &name) : absolute(name), __npath(fs::path(name))
    {
        __check();
    }

    void __check()
    {
        for (size_t pos = 0; (pos = absolute.find('\\', pos)) != std::string::npos;)
            absolute.replace(pos, 1, "/");
    }

    std::string file_name() const
    {
        return __npath.filename().string();
    }

    std::string file_format() const
    {
        return __npath.extension().string();
    }

    hio_path operator/(const std::string &name) const
    {
        bool append = absolute[absolute.length() - 1] != '/';
        return hio_path(append ? absolute + '/' + name : absolute + name);
    }

    std::string operator-(const hio_path &path) const
    {
        return fs::relative(__npath, path.__npath).generic_string();
    }
};

enum hpath_type
{
    FX_DIR,
    FX_FILE,
    FX_UNKNOWN
};

hio_path hio_open(const std::string &name);
hio_path hio_open_local(const std::string &name);
hio_path hio_parent(const hio_path &path);
void hio_del(const hio_path &path);
void hio_rename(const hio_path &path, const std::string &name);
bool hio_exists(const hio_path &path);
void hio_mkdirs(const hio_path &path);
hpath_type hio_judge(const hio_path &path);
std::vector<hio_path> hio_sub_dirs(const hio_path &path);
std::vector<hio_path> hio_sub_files(const hio_path &path);
std::vector<hio_path> hio_recurse_files(const hio_path &path);
hio_path hio_execution_path();

enum compression_level
{
    FX_COMP_NO = 0,
    FX_COMP_FASTEST = 1,
    FX_COMP_OPTIMAL = 2,
    FX_COMP_SMALLEST = 3,

    FX_COMP_RAW_READ = 8,
    FX_COMP_DCMP_READ = 9
};

std::vector<byte> hio_read_bytes(const hio_path &path, compression_level clvl = FX_COMP_RAW_READ);
void hio_write_bytes(const hio_path &path, const std::vector<byte> &data, compression_level clvl = FX_COMP_NO);
std::string hio_read_str(const hio_path &path);
void hio_write_str(const hio_path &path, const std::string &text);
std::vector<byte> hio_compress(std::vector<byte> buf, compression_level clvl = FX_COMP_OPTIMAL);
std::vector<byte> hio_decompress(std::vector<byte> buf);

} // namespace flux