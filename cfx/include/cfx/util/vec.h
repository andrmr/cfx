#pragma once

#include <cstdint>

#include "cfx/util/concepts.h"

namespace cfx::util {

template <Arithmetic T>
struct Vec;

using Vec_i = Vec<int32_t>;
using Vec_f = Vec<double>;

template <Arithmetic T>
struct Vec
{
    T x, y;
    using type = T;
};

} // namespace cfx::util