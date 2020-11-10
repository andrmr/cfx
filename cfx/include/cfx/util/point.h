#pragma once

#include <cstdint>

#include "cfx/util/concepts.h"

namespace cfx::util {

template <Arithmetic T>
struct Point;

using Point_i = Point<int32_t>;
using Point_f = Point<double>;

template <Arithmetic T>
struct Point
{
    T x, y;
    using type = T;
};

} // namespace cfx::util