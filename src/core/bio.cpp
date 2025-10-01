#include <core/bio.h>

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

binary_map bio_read(const hio_path &path)
{
    byte_buf buf = byte_buf(hio_read_bytes(path, FX_COMP_DCMP_READ));
    return bio_read_buf(buf);
}

void bio_write(const binary_map &map, const hio_path &path)
{
    hio_write_bytes(path, bio_write_buf(map).to_vector(), FX_COMP_OPTIMAL);
}

class __binparser
{
  private:
    std::string input;
    size_t pos;

    void __skipspace()
    {
        while (pos < input.size() && std::isspace(input[pos]))
            pos++;
    }

    char __cur_ch()
    {
        return (pos < input.size()) ? input[pos] : '\0';
    }

    char __nxt()
    {
        pos++;
        return __cur_ch();
    }

    binary_value __p_value();
    binary_value __p_bool();
    binary_value __p_num();
    std::string __p_key();
    binary_value __p_str();
    binary_value __p_arr();
    binary_value __p_map();

  public:
    __binparser(const std::string &str) : input(str), pos(0)
    {
        __skipspace();
        while (__cur_ch() != '{')
            __nxt();
    }

    binary_value __p()
    {
        __skipspace();
        return __p_value();
    }
};

binary_value __binparser::__p_value()
{
    __skipspace();
    char c = __cur_ch();

    if (c == 'n')
        prtlog_throw(FX_FATAL, "cannot use a null value");
    if (c == 't' || c == 'f')
        return __p_bool();
    if (c == '"')
        return __p_str();
    if (c == '[')
        return __p_arr();
    if (c == '{')
        return __p_map();
    if (c == '-' || (c >= '0' && c <= '9'))
        return __p_num();

    prtlog_throw(FX_FATAL, "unexpected character at position " + pos);
}

binary_value __binparser::__p_bool()
{
    if (input.substr(pos, 4) == "true")
    {
        pos += 4;
        return binary_value::make(true);
    }
    if (input.substr(pos, 5) == "false")
    {
        pos += 5;
        return binary_value::make(false);
    }
    prtlog_throw(FX_FATAL, "expected boolean at position " + pos);
}

binary_value __binparser::__p_num()
{
    size_t start = pos;

    if (__cur_ch() == '-')
        __nxt();

    while (pos < input.size() && std::isdigit(input[pos]))
        __nxt();

    if (__cur_ch() == '.')
    {
        __nxt();
        while (pos < input.size() && std::isdigit(input[pos]))
            __nxt();
    }

    if (__cur_ch() == 'e' || __cur_ch() == 'E')
    {
        __nxt();
        if (__cur_ch() == '+' || __cur_ch() == '-')
            __nxt();
        while (pos < input.size() && std::isdigit(input[pos]))
            __nxt();
    }

    std::string numStr = input.substr(start, pos - start);
    double value = std::stod(numStr);
    return binary_value::make(value);
}

std::string __binparser::__p_key()
{
    std::string result;
    while (pos < input.size() && __cur_ch() != '=')
    {
        char ch = __cur_ch();
        if (!std::isspace(ch))
            result += ch;
        __nxt();
    }
    return result;
}

binary_value __binparser::__p_str()
{
    if (__cur_ch() != '"')
        prtlog_throw(FX_FATAL, "expected '\"' at position " + pos);
    __nxt();

    std::string result;
    while (pos < input.size() && __cur_ch() != '"')
    {
        char c = __cur_ch();

        if (c == '\\')
        {
            __nxt();
            c = __cur_ch();
            switch (c)
            {
            case '"':
                result += '"';
                break;
            case '\\':
                result += '\\';
                break;
            case '/':
                result += '/';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u':
                __nxt();
                for (int i = 0; i < 4 && pos < input.size(); i++)
                {
                    __nxt();
                }
                result += '?';
                break;
            default:
                result += c;
                break;
            }
        }
        else
            result += c;
        __nxt();
    }

    if (__cur_ch() != '"')
        prtlog_throw(FX_FATAL, "unterminated string at position " + pos);
    __nxt();

    return binary_value::make(result);
}

binary_value __binparser::__p_arr()
{
    if (__cur_ch() != '[')
        prtlog_throw(FX_FATAL, "expected '[' at position " + pos);
    __nxt();

    binary_array result;
    __skipspace();

    if (__cur_ch() == ']')
    {
        __nxt();
        return binary_value::make(result);
    }

    while (pos < input.size())
    {
        result.data.push_back(__p_value());

        __skipspace();
        if (__cur_ch() == ']')
            break;
        if (__cur_ch() != ',')
            prtlog_throw(FX_FATAL, "expected ',' or ']' in array at position " + pos);
        __nxt();
        __skipspace();
    }

    if (__cur_ch() != ']')
        prtlog_throw(FX_FATAL, "unterminated array at position " + pos);
    __nxt();

    return binary_value::make(result);
}

binary_value __binparser::__p_map()
{
    if (__cur_ch() != '{')
        prtlog_throw(FX_FATAL, "expected '{' at position " + pos);
    __nxt();

    binary_map result;
    __skipspace();

    if (__cur_ch() == '}')
    {
        __nxt();
        return binary_value::make(result);
    }

    while (pos < input.size())
    {
        __skipspace();
        std::string key = __p_key();
        __skipspace();
        if (__cur_ch() != '=')
            prtlog_throw(FX_FATAL, "expected '=' after object key at position " + pos);
        __nxt();

        binary_value value = __p_value();
        result.data[key] = value;

        __skipspace();

        if (__cur_ch() == '}')
            break;
        if (__cur_ch() != ',')
            prtlog_throw(FX_FATAL, "expected ',' or '}' in object at position " + pos);

        __nxt();
    }

    if (__cur_ch() != '}')
        prtlog_throw(FX_FATAL, "unterminated object at position " + pos);
    __nxt();

    return binary_value::make(result);
}

binary_map bio_read_langd(const hio_path &path)
{
    binary_value result = __binparser(hio_read_str(path)).__p();
    if (result.type != __BINCVT_MAP)
        prtlog_throw(FX_FATAL, "binary root is not an object");
    return result.cast<binary_map>();
}

} // namespace flux
