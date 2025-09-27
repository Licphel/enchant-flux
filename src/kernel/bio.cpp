#include <kernel/bio.h>

namespace flux
{

void __write_map(byte_buf &buf, const binary_map &map);
void __write_array(byte_buf &buf, const binary_array &arr);

void __write_primitive(byte_buf &buf, const binary_value &v)
{
    buf.write<byte>(v.type);

    switch (v.type)
    {
    case __BINCVT_BYTE:
        buf.write<byte>(v.cast<byte>());
        break;
    case __BINCVT_SHORT:
        buf.write<short>(v.cast<short>());
        break;
    case __BINCVT_INT:
        buf.write<int>(v.cast<int>());
        break;
    case __BINCVT_LONG:
        buf.write<long>(v.cast<long>());
        break;
    case __BINCVT_FLOAT:
        buf.write<float>(v.cast<float>());
        break;
    case __BINCVT_DOUBLE:
        buf.write<double>(v.cast<double>());
        break;
    case __BINCVT_STRING_C:
        buf.write_string(v.cast<std::string>());
        break;
    case __BINCVT_BOOL:
        buf.write<bool>(v.cast<bool>());
        break;
    case __BINCVT_MAP:
        __write_map(buf, v.cast<binary_map>());
        break;
    case __BINCVT_ARRAY:
        __write_array(buf, v.cast<binary_array>());
        break;
    case __BINCVT_BUF:
        buf.write_byte_buf(v.cast<byte_buf>());
        break;
    }
}

void __write_map(byte_buf &buf, const binary_map &map)
{
    for (auto kv : map.data)
    {
        std::string str = kv.first;
        binary_value v = kv.second;

        __write_primitive(buf, v);
        buf.write_string(str);
    }

    buf.write<byte>(__BINCVT_EOF);
}

void __write_array(byte_buf &buf, const binary_array &arr)
{
    buf.write<size_t>(arr.size());
    for (auto bv : arr.data)
        __write_primitive(buf, bv);
}

binary_map __read_map(byte_buf &buf);
binary_array __read_array(byte_buf &buf);

binary_value __read_primitive(byte_buf &buf)
{
    byte id = buf.read<byte>();

    if (id == __BINCVT_EOF)
        return {__BINCVT_EOF, std::any(0)};

    switch (id)
    {
    case __BINCVT_BYTE:
        return binary_value::make(buf.read<byte>());
    case __BINCVT_SHORT:
        return binary_value::make(buf.read<short>());
    case __BINCVT_INT:
        return binary_value::make(buf.read<int>());
    case __BINCVT_LONG:
        return binary_value::make(buf.read<long>());
    case __BINCVT_FLOAT:
        return binary_value::make(buf.read<float>());
    case __BINCVT_DOUBLE:
        return binary_value::make(buf.read<double>());
    case __BINCVT_STRING_C:
        return binary_value::make(buf.read_string());
    case __BINCVT_BOOL:
        return binary_value::make(buf.read<bool>());
    case __BINCVT_MAP:
        return binary_value::make(__read_map(buf));
    case __BINCVT_ARRAY:
        return binary_value::make(__read_array(buf));
    case __BINCVT_BUF:
        return binary_value::make(buf.read_byte_buf());
    }

    prtlog_throw(FX_FATAL, "unknown binary id.");
}

binary_map __read_map(byte_buf &buf)
{
    binary_map map;
    while (true)
    {
        binary_value bv = __read_primitive(buf);

        if (bv.type == __BINCVT_EOF)
            break;

        std::string str = buf.read_string();
        map.data[str] = bv;
    }
    return map;
}

binary_array __read_array(byte_buf &buf)
{
    size_t size = buf.read<size_t>();
    binary_array arr;
    while (size-- > 0)
        arr.data.push_back(__read_primitive(buf));
    return arr;
}

binary_map bio_read_buf(byte_buf &v)
{
    return __read_map(v);
}

byte_buf bio_write_buf(const binary_map &map)
{
    byte_buf buf;
    __write_map(buf, map);
    return buf;
}

binary_map bio_read(const hpath &path)
{
    byte_buf buf = byte_buf(read_bytes(path, FX_COMP_DCMP_READ));
    return bio_read_buf(buf);
}

void bio_write(const binary_map &map, const hpath &path)
{
    write_bytes(path, bio_write_buf(map).to_vector(), FX_COMP_OPTIMAL);
}

} // namespace flux
