#pragma once
#include <linc/Include.hpp>
#include <linc/system/Files.hpp>
#include <linc/lexer/Token.hpp>

namespace linc
{
    class Code final 
    {
    public:
        Code() = delete;
        
        struct Line final
        {
            std::string text;
            std::string file;
            std::size_t line;
        };

        struct Character final
        {
            char character;
            std::string file;
            std::size_t line;

            inline operator char() const { return character; }
            inline Token::Info getInfo() const { return Token::Info{.file = file, .line = line}; }
        };

        using Source = std::vector<Line>;
        using SourceSize = std::vector<Line>::size_type;
        using StringSize = std::string::size_type;

        static void append(Source& source, const Source& new_source)
        {
            source.insert(source.end(), new_source.begin(), new_source.end());
        }

        [[nodiscard]] static Source toSource(std::string raw_source, const std::string& filename = "")
        {
            Source result{};
            std::string buffer{};
            std::size_t line_count{1ull};
            
            raw_source.push_back('\n');

            for(char c : raw_source)
            {
                buffer.push_back(c);
                if(c == '\n')
                {
                    result.push_back(Line{.text = buffer, .file = filename, .line = ++line_count});
                    buffer = {};
                }
            }
                

            return result;
        }

        [[nodiscard]] static std::optional<Character> peek(const Source& source, StringSize character_index, StringSize line_index, StringSize offset)
        {
            if(line_index >= source.size())
                return std::nullopt;
            else if(character_index + offset >= source[line_index].text.size())
            {
                if(line_index + 1 < source.size())
                    return Character {
                        .character = source[line_index + 1].text[character_index + offset - source[line_index].text.size()],
                        .file = source[line_index].file,
                        .line = source[line_index].line
                    };

                else return std::nullopt;
            }

            return Character{
                .character = source[line_index].text[character_index + offset],
                .file = source[line_index].file,
                .line = source[line_index].line
            };
        }

        [[nodiscard]] static Character consume(const Source& source, StringSize& character_index, StringSize& line_index)
        {
            auto pre_character_index = character_index;
            auto pre_line_index = line_index;

            if(++character_index >= source[line_index].text.size())
            {
                character_index = {};
                ++line_index;
            }

            return Character{
                .character = source[pre_line_index].text[pre_character_index],  
                .file = source[pre_line_index].file,
                .line = source[pre_line_index].line
            };
        }
    };
}