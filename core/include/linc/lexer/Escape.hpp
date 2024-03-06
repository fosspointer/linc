#pragma once
#include <linc/Include.hpp>

namespace linc
{
    class Escape final 
    {
    public:
        Escape() = delete;
        static std::optional<char> get(char escape_character);
    private:
        static std::unordered_map<char, char> s_escapeCharacterMap;
    };
}