#pragma once
#include <core/bin.h>
#include <core/hio.h>
#include <core/buffer.h>

namespace flux
{

binary_map bio_read_buf(byte_buf &v);
byte_buf bio_write_buf(const binary_map &map);
binary_map bio_read(const hio_path &path);
void bio_write(const binary_map &map, const hio_path &path);
// read a script-form binary map (like json, but not the same).
binary_map bio_read_langd(const hio_path& path);

} // namespace flux
