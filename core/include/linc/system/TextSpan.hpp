#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Code.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Class representing a color-annotated line of source code.
    struct TextSpan final
    {
        std::string::size_type lineIndex, spanStart, spanEnd;

        /// @brief Get the current object's annotated line of a specified source code.
        /// @param source The source code to use.
        /// @param highlight_color The color of the highlight/annotation.
        /// @return The source code line with highlights in ANSI string format.
        std::string get(Code::Source& source, Colors::Color highlight_color = Colors::Color::Red) const
        {
            auto line = source.at(lineIndex).text;
            auto format = linc::Logger::format("$:$:$:$:$", line.substr(0ul, spanStart), Colors::toANSI(highlight_color),
                line.substr(spanStart, spanEnd - spanStart), Colors::toANSI(Colors::Color::Default),
                line.substr(spanEnd));

            return Code::trim(format);
        }
    };
}