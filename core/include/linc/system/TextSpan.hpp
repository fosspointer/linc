#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/Printable.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Code.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Class representing a color-annotated line of source code.
    struct TextSpan final
    {
        std::string::size_type lineStart, lineEnd, spanStart, spanEnd;
        std::string file;

        bool operator==(const TextSpan&) const = default;
        bool operator!=(const TextSpan&) const = default;

        /// @brief Get the current object's annotated lines of a specified source code.
        /// @param source The source code to use.
        /// @param highlight_color The color of the highlight/annotation.
        /// @return The source code lines with highlights in ANSI string format.
        std::string get(Code::Source& source, Colors::Color highlight_color = Colors::Red) const
        {
            std::string result;
            if(lineStart > source.size() || lineStart > lineEnd)
                throw LINC_EXCEPTION_ILLEGAL_VALUE(lineStart);
            else if(lineEnd > source.size())
                throw LINC_EXCEPTION_ILLEGAL_VALUE(lineEnd);

            std::size_t startIndex{-1ul}, endIndex{-1ul};
            for(std::size_t i{0ul}; i < source.size(); ++i)
            {
                if(source[i].line == lineStart && source[i].file == file)
                    startIndex = i;
                if(source[i].line == lineEnd && source[i].file == file)
                {
                    endIndex = i;
                    break;
                }
            }

            if(startIndex == -1ul) throw LINC_EXCEPTION_ILLEGAL_STATE(startIndex);
            else if(endIndex == -1ul) throw LINC_EXCEPTION_ILLEGAL_STATE(endIndex);

            for(std::size_t i{startIndex}; i <= endIndex; ++i)
            {
                auto line = source.at(i).text;
                std::string format;

                if(startIndex == endIndex)
                {
                    if(spanEnd < spanStart) throw LINC_EXCEPTION_ILLEGAL_VALUE(spanStart);
                    format = linc::Logger::format("$:$:$:$:$", line.substr(0ul, spanStart), Colors::toANSI(highlight_color),
                        line.substr(spanStart, spanEnd - spanStart), Colors::toANSI(Colors::Reset), line.substr(spanEnd));
                }
                else if(i == startIndex)
                {
                    format = linc::Logger::format("$:$:$:$", line.substr(0ul, spanStart), Colors::toANSI(highlight_color),
                        line.substr(spanStart), Colors::toANSI(Colors::Reset));
                }
                else if(i == endIndex)
                {
                    format = linc::Logger::format("$:$:$:$", Colors::toANSI(highlight_color),
                        line.substr(0ul, spanEnd), Colors::toANSI(Colors::Reset),
                        line.substr(spanEnd));
                }
                else
                {
                    format = linc::Logger::format("$:$:$", Colors::toANSI(highlight_color),
                        line, Colors::toANSI(Colors::Reset));
                }

                result += Code::trim(format);

                if(i != lineEnd - 1ul) result.push_back('\n');
            }

            return result;
        }

        static TextSpan fromTokenInfo(const Token::Info& token_info)
        {
            return TextSpan{.lineStart = token_info.line, .lineEnd = token_info.line, .spanStart = token_info.characterStart, .spanEnd = token_info.characterEnd, .file = token_info.file};
        }

        static TextSpan fromTokenInfoRange(const Token::Info& from, const Token::Info& to)
        {
            if(from.file != to.file)
                throw LINC_EXCEPTION_ILLEGAL_STATE(to.file);

            return TextSpan{.lineStart = from.line, .lineEnd = to.line, .spanStart = from.characterStart, .spanEnd = to.characterEnd, .file = from.file};
        }
    };
}