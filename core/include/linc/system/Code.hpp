#pragma once
#include <linc/Include.hpp>
#include <linc/system/Files.hpp>
#include <linc/lexer/Token.hpp>

namespace linc
{
    /// @brief Static code utility class.
    class Code final 
    {
    public:
        Code() = delete;
        
        static std::string trim(const std::string& text)
        {
            if(text.empty()) return text;

            std::string::size_type start = 0ul, end = text.size() - 1ul;

            while(start < text.size() && std::isspace(text[start])) ++start;
            while(end > start && std::isspace(text[end])) --end;

            return text.substr(start, end - start + 1ul);
        }

        /// @brief Structure representation of a single line of code, along with the corresponding file and line it belongs to.
        struct Line final
        {
            std::string text, file;
            std::size_t line;
        };

        /// @brief Structure representation of a character, along with the corresponding line and file it belongs to.
        /// Note: not all characters are represented using this format, as that would be too memory intensive.
        struct Character final
        {
            char character;
            std::string file;
            std::size_t line, characterIndex;

            inline operator char() const { return character; }
            inline Token::Info getInfo() const { return Token::Info{.file = file, .line = line, .characterIndex = characterIndex}; }
        };

        /// @brief Representation of source code, as a dynamic array of lines.
        using Source = std::vector<Line>;

        /// @brief The appropriate size type for the source code array type.
        using SourceSize = Source::size_type;

        /// @brief The appropriate size type for a standard string.
        using StringSize = std::string::size_type;

        /// @brief Append a source to its destination, similar to how it's done with strings.
        /// @param source The destination source.
        /// @param new_source The source to be appended.
        static void append(Source& source, const Source& new_source)
        {
            source.insert(source.end(), new_source.begin(), new_source.end());
        }

        /// @brief Convert raw source code to the 'source' structure in use. Use the given filenames and compute the line numbers for each line.
        /// @param raw_source The original, raw source code.
        /// @param filepath The filepath of the source file (can be arbitrary if from a buffer, e.g. from stdin).
        /// @return The resulting source code.
        [[nodiscard]] static Source toSource(std::string raw_source, const std::string& filepath = "")
        {
            Source result{};
            std::string buffer{};
            std::size_t line_count{};
            raw_source.push_back('\n');

            for(char c: raw_source)
            {
                buffer.push_back(c);
                if(c == '\n')
                {
                    result.push_back(Line{.text = buffer, .file = filepath, .line = ++line_count});
                    buffer = {};
                }
            }

            return result;
        }

        /// @brief Compute the character at a specified offset past the given indices of a source structure, which may also be out of bounds for the
        /// current line. If so, the following line is checked for accordingly.
        /// @param source The source code to test for.
        /// @param character_index The current character index for the current line.
        /// @param line_index The current line index.
        /// @param offset The offset (in characters) past the current indices.
        /// @return The structure corresponding to the resulting character.
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
                        .line = source[line_index].line,
                        .characterIndex = character_index
                    };

                else return std::nullopt;
            }

            return Character{
                .character = source[line_index].text[character_index + offset],
                .file = source[line_index].file,
                .line = source[line_index].line,
                .characterIndex = character_index
            };
        }

        /// @brief Compute the immediate next character past the given indices of a source structure, then increment them accordingly. 
        /// @param source The source code to use.
        /// @param character_index The current character index for the current line.
        /// @param line_index The current line index.
        /// @return The structure corresponding to the consumed, now previous character.
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
                .line = source[pre_line_index].line,
                .characterIndex = character_index
            };
        }
    };
}