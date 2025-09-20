#pragma once
#include <memory>

namespace flux
{

// Defines some abbreviations.
typedef unsigned char byte;
template <class T> using unique = std::unique_ptr<T>;
template <class T> using shared = std::shared_ptr<T>;
template <class T> using weak = std::weak_ptr<T>;

} // namespace flux