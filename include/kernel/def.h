#pragma once
#include <memory>

#define FLUX_VERSION "v1.1.1"

// I prefer the y-axis to point upwards, but modern graphics APIs usually have
// the y-axis pointing downwards. You can undefine this to make the y-axis point upwards.
// However, it is not guaranteed that all parts of the engine will respect this setting.
#define FX_Y_IS_DOWN

namespace flux
{

// defines some abbreviations.
typedef unsigned char byte;
// represent a UTF-32 code point.
typedef char32_t general_char;
template <class T> using unique = std::unique_ptr<T>;
template <class T> using shared = std::shared_ptr<T>;
template <class T> using weak = std::weak_ptr<T>;

} // namespace flux