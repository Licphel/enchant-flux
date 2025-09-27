#include <kernel/hio.h>
#include <kernel/log.h>

#define BROTLI_IMPLEMENTATION
#include <brotli/encode.h>
#include <brotli/decode.h>

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
        prtlog_throw(FX_FATAL, "Brotli decoder create failed");

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
            prtlog_throw(FX_FATAL, "Brotli decoder error");
    }
    BrotliDecoderDestroyInstance(st);
    return dst;
}

std::vector<byte> read_bytes(const hpath &path, compression_level clvl)
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

    std::vector<byte> dec = brotli_decompress(raw);
    if (dec.empty() && !raw.empty())
        return raw;
    return dec;
}

void write_bytes(const hpath &path, const std::vector<byte> &data, compression_level clvl)
{
    if (!exists(path))
        mkdirs(path);

    std::vector<byte> out;
    switch (clvl)
    {
    case FX_COMP_NO:
        out = data;
        break;
    case FX_COMP_FASTEST:
        out = brotli_compress(data, 1);
        break;
    case FX_COMP_OPTIMAL:
        out = brotli_compress(data, 6);
        break;
    case FX_COMP_SMALLEST:
        out = brotli_compress(data, 11);
        break;
    default:
        prtlog_throw(FX_FATAL, "unsupported compression level {}", (int)clvl);
    }

    std::ofstream file(path.__npath, std::ios::binary);
    if (!file)
        prtlog_throw(FX_FATAL, "cannot open {} for write", path.absolute);
    file.write(reinterpret_cast<const char *>(out.data()), out.size());
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