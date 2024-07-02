#include <linc/system/Reporting.hpp>
#include <linc/lexer/Escape.hpp>

#define LINC_ESCAPE_CHARACTER_MAP_PAIR(first, second) std::pair<char, char>(first, second)

namespace linc
{
    std::unordered_map<char, char> Escape::s_escapeCharacterMap = {
        LINC_ESCAPE_CHARACTER_MAP_PAIR('a', '\a'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('b', '\b'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('e', '\e'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('f', '\f'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('n', '\n'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('r', '\r'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('t', '\t'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('v', '\v'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('\\', '\\'),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('\'', '\''),
        LINC_ESCAPE_CHARACTER_MAP_PAIR('"', '"'),
    };

    std::optional<char> Escape::get(char escape_character)
    {
        auto find = s_escapeCharacterMap.find(escape_character);

        if(find != s_escapeCharacterMap.end())
            return find->second;

        Reporting::push(Reporting::Report{
            .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
            .message = linc::Logger::format("Invalid escape character '\\$'.", escape_character)
        });

        return std::nullopt;
    }
}