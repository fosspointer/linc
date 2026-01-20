#pragma once
#include <unordered_map>
#include <optional>

namespace linc
{
    class Escape final
    {
    public:
        Escape() = delete;

        std::optional<char> get(char escape_character);
    private:
        static std::unordered_map<char, char> s_escapeMap;
    };
}
