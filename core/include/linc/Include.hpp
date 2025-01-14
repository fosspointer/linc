#pragma once
#include <cstdio>
#include <cstdint>
#ifdef __clang__
namespace std
{
    typedef float float32_t;
    typedef double float64_t;
    static_assert(sizeof(float) == 4ul);
    static_assert(sizeof(double) == 8ul);
}
#else
#include <stdfloat>
#endif
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <array>
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <utility>
#include <memory>
#include <functional>
#include <csignal>