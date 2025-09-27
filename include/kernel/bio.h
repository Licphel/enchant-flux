#pragma once
#include <kernel/bin.h>
#include <kernel/hio.h>
#include <kernel/buf.h>

namespace flux
{

binary_map bio_read_buf(byte_buf& v);
byte_buf bio_write_buf(const binary_map& map);
binary_map bio_read(const hpath& path);
void bio_write(const binary_map& map, const hpath& path);
    
} // namespace flux
