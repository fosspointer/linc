#pragma once
#include <cstddef>

namespace linc
{
    class Numbers final
    {
    public:
        Numbers() = delete;
        static std::size_t getBaseByDescriptor(char descriptor);
        constexpr static const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr static std::size_t defaultBase = 10ul;
    };
}
