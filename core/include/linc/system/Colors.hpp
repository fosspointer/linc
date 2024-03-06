#pragma once

namespace linc
{
    /// @brief Color utility static class
    class Colors final
    {
    public:
        Colors() = delete;

        /// @brief Enum representing basic ANSI foreground colors
        enum class Color: char
        {
            Default, Black, Red, Green, Yellow, Blue, Purple, Cyan, White
        };

        /// @brief Convert an enumerator to its corresponding ANSI color-sequence, such that following text is of the specified color (until re-specified). 
        /// @param color The enumerator corresponding to the selected color.
        /// @return The resulting ANSI color-sequence.
        static std::string toANSI(Color color)
        {
            static const std::string black  = "\e[0;30m";
            static const std::string red    = "\e[0;31m";
            static const std::string green  = "\e[0;32m";
            static const std::string yellow = "\e[0;33m";
            static const std::string blue   = "\e[0;34m";
            static const std::string purple = "\e[0;35m";
            static const std::string cyan   = "\e[0;36m";
            static const std::string white  = "\e[0;37m";
            static const std::string _default  = "\e[0m";

            switch(color)
            {
            case Color::Black:  return black;
            case Color::Red:    return red;
            case Color::Green:  return green;
            case Color::Yellow: return yellow;
            case Color::Blue:   return blue;
            case Color::Purple: return purple;
            case Color::Cyan:   return cyan;
            case Color::White:  return white;
            default:
                return _default;
            }
        }
    };
}