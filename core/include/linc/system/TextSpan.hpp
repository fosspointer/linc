#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/Colors.hpp>
#include <linc/system/Code.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct TextSpan final
    {
        std::string::size_type lineIndex, spanStart, spanEnd;

        std::string get(Code::Source& source, Colors::Color color = Colors::Color::Red) const
        {
            auto line = source.at(lineIndex).text;
            auto format = linc::Logger::format("$:$:$:$:$", line.substr(0ul, spanStart), Colors::toANSI(color),
                line.substr(spanStart, spanEnd - spanStart), Colors::toANSI(Colors::Color::Default),
                line.substr(spanEnd));

            return Code::trim(format);
        }
    };
}