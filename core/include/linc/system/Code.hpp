#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <cstdio>
#include <linc/lexer/Token.hpp>

namespace linc
{
    class Code final
    {
    public:
        Code() = delete;

        struct Line final
        {
            std::string_view text;
            std::size_t file, line;
        };
        using Source = std::vector<Line>;

        struct Character final
        {
            char character;
            std::size_t file, line, index;

            inline operator char() const { return character; }
            inline Token::Info getInfo() const { return Token::Info{.file = file, .line = line, .characterStart = index, .characterEnd = index + 1ul}; }
        };

        [[nodiscard]] static inline Source toSource(std::string_view filepath)
        {
            Source result;

            auto filepath_string = std::string{filepath};
            std::FILE* file = std::fopen(filepath_string.c_str(), "r");
            std::size_t length;
            ssize_t bytes_read;
            char* current_line = nullptr;

            std::size_t line_index{0ul};

            if(!file)
                return Source{};

            while((bytes_read = getline(&current_line, &length, file)) != -1)
            {
                result.push_back(Line{.text = std::string_view{current_line}, .file = 0ul, .line = ++line_index});
                current_line = nullptr;
            } 

            std::fclose(file);
            return result;
        }


        [[nodiscard]] static Source toSource(std::string_view raw_source, std::string_view filepath)
        {
            Source result;
            std::size_t start_line_character_index{0ul}, line_index{0ul};
            
            for(std::size_t current_character_index{0ul}; current_character_index < raw_source.size(); ++current_character_index)
                if(raw_source[current_character_index] == '\n')
                {
                    result.push_back(Line{.text = raw_source.substr(start_line_character_index, current_character_index - start_line_character_index), .file = 0ul, .line = ++line_index});
                    start_line_character_index = current_character_index + 1ul;
                    continue;
                }

            if(start_line_character_index < raw_source.size())
                result.push_back(Line{.text = raw_source.substr(start_line_character_index), .file = 0ul, .line = ++line_index});

            return result;
        }

        [[nodiscard]] static std::optional<Character> peek(const Source& source, std::string::size_type character_index, std::string::size_type line_index, std::string::size_type offset)
        {
            if(line_index >= source.size())
                return std::nullopt;
            else if(character_index + offset < source[line_index].text.size())
                return Character{
                    .character = source[line_index].text[character_index + offset],
                    .file = source[line_index].file,
                    .line = source[line_index].line,
                    .index = character_index
                };
            else if(line_index + 1ul < source.size())
                return Character{
                    .character = source[line_index + 1ul].text[character_index + offset - source[line_index].text.size()],
                    .file = source[line_index].file,
                    .line = source[line_index].line,
                    .index = character_index
                };
            else return std::nullopt;
        }

        [[nodiscard]] static Character consume(const Source& source, std::string::size_type& character_index, std::string::size_type& line_index)
        {
            auto character_index_before = character_index;
            auto line_index_before = line_index;

            if(++character_index >= source[line_index].text.size())
            {
                character_index = {};
                ++line_index;
            }

            return Character{
                .character = source[line_index_before].text[character_index_before],
                .file = source[line_index_before].file,
                .line = source[line_index_before].line,
                .index = character_index_before
            };
        }
    };
}
