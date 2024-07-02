#pragma once
#include <linc/system/Exception.hpp>
#include <linc/Include.hpp>

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

        /// @brief Push the specified color onto the stack.
        /// @param color Color to push.
        /// @return The specified color converted to its ANSI sequence equivalent.
        inline static std::string push(Color color)
        {
            s_colorStack.push(color);
            return toANSI(color);
        }

        /// @brief Pop the top of the color-stack.
        /// @return The stack top after removing the color, converted to its ANSI sequence equivalent.
        inline static std::string pop()
        {
            if(s_colorStack.empty())
                throw LINC_EXCEPTION("Tried to call pop when the color stack was empty.");

            s_colorStack.pop();
            return toANSI(getCurrentColor());
        }

        /// @brief Convert an enumerator to its corresponding ANSI color-sequence, such that following text is of the specified color (until re-specified). 
        /// @param color The enumerator corresponding to the selected color.
        /// @return The resulting ANSI color-sequence.
        [[nodiscard]] static std::string toANSI(Color color)
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

        /// @brief Get the color at the top of the stack. If the stack is empty, return the default color.
        /// @return The enumerator corresponding to the current color.
        [[nodiscard]] inline static Color getCurrentColor() { return s_colorStack.empty()? Color::Default: s_colorStack.top(); }
    private:
        static std::stack<Color> s_colorStack;
    };
}