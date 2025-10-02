#include <core/hio.h>
#include <core/log.h>

#define BROTLI_IMPLEMENTATION
#include <brotli/encode.h>
#include <brotli/decode.h>

namespace flux
{

hio_path::hio_path() = default;

hio_path::hio_path(const std::string &name) : absolute(name), __npath(fs::path(name))
{
    __check();
}

void hio_path::__check()
{
    for (size_t pos = 0; (pos = absolute.find('\\', pos)) != std::string::npos;)
        absolute.replace(pos, 1, "/");
}

std::string hio_path::file_name() const
{
    return __npath.filename().string();
}

std::string hio_path::file_format() const
{
    return __npath.extension().string();
}

hio_path hio_path::operator/(const std::string &name) const
{
    bool append = absolute[absolute.length() - 1] != '/';
    return hio_path(append ? absolute + '/' + name : absolute + name);
}

std::string hio_path::operator-(const hio_path &path) const
{
    return fs::relative(__npath, path.__npath).generic_string();
}

hio_path hio_open(const std::string &name)
{
    return hio_path(name);
}

hio_path hio_open_local(const std::string &name)
{
    return hio_execution_path() / name;
}

hio_path hio_parent(const hio_path &path)
{
    return {path.__npath.parent_path().string()};
}

void hio_del(const hio_path &path)
{
    fs::remove(path.__npath);
}

void hio_rename(const hio_path &path, const std::string &name)
{
    fs::rename(path.__npath, name);
}

bool hio_exists(const hio_path &path)
{
    return fs::exists(path.__npath);
}

void hio_mkdirs(const hio_path &path)
{
    fs::create_directories(hio_judge(path) == FX_DIR ? path.__npath : hio_parent(path).__npath);
    if (hio_judge(path) == FX_FILE)
    {
        std::ofstream stream(path.__npath);
        stream.close();
    }
}

hpath_type hio_judge(const hio_path &path)
{
    if (fs::is_directory(path.__npath))
        return FX_DIR;
    if (fs::is_regular_file(path.__npath))
        return FX_FILE;
    return FX_UNKNOWN;
}

std::vector<hio_path> hio_sub_dirs(const hio_path &path)
{
    std::vector<hio_path> paths;
    for (auto k : fs::directory_iterator(path.__npath))
    {
        if (k.is_directory())
            paths.push_back(hio_path(k.path().string()));
    }
    return paths;
}

std::vector<hio_path> hio_sub_files(const hio_path &path)
{
    std::vector<hio_path> paths;
    for (auto k : fs::directory_iterator(path.__npath))
    {
        if (k.is_regular_file())
            paths.push_back(hio_path(k.path().string()));
    }
    return paths;
}

std::vector<hio_path> hio_recurse_files(const hio_path &path)
{
    std::vector<hio_path> paths;
    for (auto k : fs::recursive_directory_iterator(path.__npath))
    {
        if (k.is_regular_file())
            paths.push_back(hio_path(k.path().string()));
    }
    return paths;
}

hio_path hio_execution_path()
{
    return hio_path(fs::current_path().string()) / "run";
}

static std::vector<byte> brotli_compress(const std::vector<byte> &src, int quality)
{
    if (src.empty())
        return {};
    size_t max_sz = BrotliEncoderMaxCompressedSize(src.size());
    std::vector<byte> out(max_sz);
    size_t encoded_sz = max_sz;
    if (BROTLI_TRUE != BrotliEncoderCompress(quality, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, src.size(),
                                             src.data(), &encoded_sz, out.data()))
        prtlog_throw(FX_FATAL, "Brotli encoder failed");
    out.resize(encoded_sz);
    return out;
}

static std::vector<byte> brotli_decompress(const std::vector<byte> &src)
{
    if (src.empty())
        return {};
    std::vector<byte> dst;
    size_t avail_in = src.size();
    const byte *nxt_in = src.data();
    BrotliDecoderState *st = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!st)
        prtlog_throw(FX_FATAL, "brotli decoder create failed");

    for (;;)
    {
        byte buf[64 * 1024];
        size_t avail_out = sizeof(buf);
        byte *nxt_out = buf;
        auto rc = BrotliDecoderDecompressStream(st, &avail_in, &nxt_in, &avail_out, &nxt_out, nullptr);
        size_t produced = sizeof(buf) - avail_out;
        if (produced)
            dst.insert(dst.end(), buf, buf + produced);
        if (rc == BROTLI_DECODER_RESULT_SUCCESS)
            break;
        if (rc == BROTLI_DECODER_RESULT_ERROR)
            prtlog_throw(FX_FATAL, "brotli decoder error");
    }
    BrotliDecoderDestroyInstance(st);
    return dst;
}

std::vector<byte> hio_read_bytes(const hio_path &path, compression_level clvl)
{
    std::ifstream file(path.__npath, std::ios::binary | std::ios::ate);
    if (!file)
        prtlog_throw(FX_FATAL, "cannot find {}", path.absolute);
    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<byte> raw(len);
    file.read(reinterpret_cast<char *>(raw.data()), len);
    if (!file)
        prtlog_throw(FX_FATAL, "short read in {}", path.absolute);

    if (clvl == FX_COMP_RAW_READ)
        return raw;

    return hio_decompress(raw);
}

void hio_write_bytes(const hio_path &path, const std::vector<byte> &data, compression_level clvl)
{
    if (!hio_exists(path))
        hio_mkdirs(path);
    auto out = hio_compress(data, clvl);
    std::ofstream file(path.__npath, std::ios::binary);
    if (!file)
        prtlog_throw(FX_FATAL, "cannot open {} for write", path.absolute);
    file.write(reinterpret_cast<const char *>(out.data()), out.size());
}

std::string hio_read_str(const hio_path &path)
{
    auto raw = hio_read_bytes(path);
    return std::string(reinterpret_cast<const char *>(raw.data()), raw.size());
}

void hio_write_str(const hio_path &path, const std::string &text)
{
    hio_write_bytes(path, std::vector<byte>(reinterpret_cast<const byte *>(text.data()),
                                            reinterpret_cast<const byte *>(text.data() + text.size())));
}

std::vector<byte> hio_compress(std::vector<byte> buf, compression_level clvl)
{
    std::vector<byte> out;
    switch (clvl)
    {
    case FX_COMP_NO:
        out = buf;
        break;
    case FX_COMP_FASTEST:
        out = brotli_compress(buf, 1);
        break;
    case FX_COMP_OPTIMAL:
        out = brotli_compress(buf, 6);
        break;
    case FX_COMP_SMALLEST:
        out = brotli_compress(buf, 11);
        break;
    default:
        prtlog_throw(FX_FATAL, "unsupported compression level {}", (int)clvl);
    }
    return out;
}

std::vector<byte> hio_decompress(std::vector<byte> buf)
{
    return brotli_decompress(buf);
}

} // namespace flux