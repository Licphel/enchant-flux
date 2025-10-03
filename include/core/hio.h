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

    hio_path();
    hio_path(const std::string &name);

    void __check();

    // returns the file name with extension.
    std::string file_name() const;
    // returns the file extension with the dot, like ".txt".
    std::string file_format() const;
    hio_path operator/(const std::string &name) const;
    // get the relative path, say 'minus' the #path from #this path.
    // for example, "C:/dir/file.txt" - "C:/dir" = "file.txt".
    std::string operator-(const hio_path &path) const;
};

enum hpath_type
{
    FX_DIR,
    FX_FILE,
    FX_UNKNOWN
};

// open a absolute path.
hio_path hio_open(const std::string &name);
// open a path relative to the execution path of the executable file.
hio_path hio_open_local(const std::string &name);
hio_path hio_parent(const hio_path &path);
void hio_del(const hio_path &path);
void hio_rename(const hio_path &path, const std::string &name);
bool hio_exists(const hio_path &path);
// if the path is a file, create its parent directories and an empty file.
// if the path is a directory, create it and its parent directories.
void hio_mkdirs(const hio_path &path);
// get the type of the path.
hpath_type hio_judge(const hio_path &path);
std::vector<hio_path> hio_sub_dirs(const hio_path &path);
// get all files in the directory, but not in its sub-directories.
std::vector<hio_path> hio_sub_files(const hio_path &path);
// get all files in the directory and its sub-directories, and, so on, recursively.
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

// note: if you want to read a compressed file, use FX_COMP_DCMP_READ instead of FX_COMP_RAW_READ.
std::vector<byte> hio_read_bytes(const hio_path &path, compression_level clvl = FX_COMP_RAW_READ);
void hio_write_bytes(const hio_path &path, const std::vector<byte> &data, compression_level clvl = FX_COMP_NO);
std::string hio_read_str(const hio_path &path);
void hio_write_str(const hio_path &path, const std::string &text);
std::vector<byte> hio_compress(std::vector<byte> buf, compression_level clvl = FX_COMP_OPTIMAL);
std::vector<byte> hio_decompress(std::vector<byte> buf);

} // namespace flux