#pragma once

#include <type_traits>

namespace cfx::util {

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept CharAssignable = std::is_assignable_v<T, const char*>;

} // namespace cfx::util