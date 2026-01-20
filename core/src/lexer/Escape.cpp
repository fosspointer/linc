#include <linc/lexer/Escape.hpp>

namespace linc
{
    std::unordered_map<char, char> Escape::s_escapeMap{
        std::pair{'a', '\a'},
        std::pair{'b', '\b'},
        std::pair{'e', '\x1B'},
        std::pair{'f', '\f'},
        std::pair{'n', '\n'},
        std::pair{'r', '\r'},
        std::pair{'t', '\t'},
        std::pair{'v', '\v'},
        std::pair{'\\', '\\'},
        std::pair{'\'', '\''},
        std::pair{'"', '"'}
    };

    std::optional<char> Escape::get(char escape_character)
    {
        auto find = s_escapeMap.find(escape_character);

        if(find != s_escapeMap.end())
            return find->second;

        return std::nullopt;
    }
}
