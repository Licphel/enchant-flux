#include <kernel/hio.h>
#include <kernel/common.h>

namespace flux
{

hpath open(const std::string &name)
{
    return hpath(name);
}

hpath open_local(const std::string &name)
{
    return execution_path() / name;
}

hpath parent(const hpath &path)
{
    return {path.__npath.parent_path().string()};
}

void del(const hpath &path)
{
    fs::remove(path.__npath);
}

void rename(const hpath &path, const std::string &name)
{
    fs::rename(path.__npath, name);
}

bool exists(const hpath &path)
{
    return fs::exists(path.__npath);
}

void mkdirs(const hpath &path)
{
    fs::create_directories(judge(path) == FX_DIR ? path.__npath : parent(path).__npath);
    if (judge(path) == FX_FILE)
    {
        std::ofstream stream(path.__npath);
        stream.close();
    }
}

hpath_type judge(const hpath &path)
{
    if (fs::is_directory(path.__npath))
        return FX_DIR;
    if (fs::is_regular_file(path.__npath))
        return FX_FILE;
    return FX_UNKNOWN;
}

std::vector<hpath> sub_dirs(const hpath &path)
{
    std::vector<hpath> paths;
    for (auto k : fs::directory_iterator(path.__npath))
    {
        if (k.is_directory())
            paths.push_back(hpath(k.path().string()));
    }
    return paths;
}

std::vector<hpath> sub_files(const hpath &path)
{
    std::vector<hpath> paths;
    for (auto k : fs::directory_iterator(path.__npath))
    {
        if (k.is_regular_file())
            paths.push_back(hpath(k.path().string()));
    }
    return paths;
}

std::vector<hpath> recurse_files(const hpath &path)
{
    std::vector<hpath> paths;
    for (auto k : fs::recursive_directory_iterator(path.__npath))
    {
        if (k.is_regular_file())
            paths.push_back(hpath(k.path().string()));
    }
    return paths;
}

hpath execution_path()
{
    return hpath(fs::current_path().string()) / "run";
}

std::vector<byte> read_bytes(const hpath &path)
{
    std::ifstream file(path.__npath, std::ios::binary | std::ios::ate);
    if (!file)
        prtlog_throw(FATAL, "cannot find {}", path.absolute);

    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<byte> buffer(len);
    file.read(reinterpret_cast<char *>(buffer.data()), len);
    if (!file)
        prtlog_throw(FATAL, "short read in {}", path.absolute);

    return buffer;
}

void write_bytes(const hpath &path, const std::vector<byte> &data)
{
    if (!exists(path))
        mkdirs(path);
    std::ofstream file(path.__npath, std::ios::binary);
    if (!file)
        prtlog_throw(FATAL, "cannot open {} for write", path.absolute);
    file.write(reinterpret_cast<const char *>(data.data()), data.size());
}

std::string read_str(const hpath &path)
{
    auto raw = read_bytes(path);
    return std::string(reinterpret_cast<const char *>(raw.data()), raw.size());
}

void write_str(const hpath &path, const std::string &text)
{
    write_bytes(path, std::vector<byte>(reinterpret_cast<const byte *>(text.data()),
                                        reinterpret_cast<const byte *>(text.data() + text.size())));
}

} // namespace flux