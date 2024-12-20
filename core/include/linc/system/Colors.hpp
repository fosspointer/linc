#pragma once
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
        static std::string push(Color color);

        /// @brief Pop the top of the color-stack.
        /// @return The stack top after removing the color, converted to its ANSI sequence equivalent.
        static std::string pop();

        /// @brief Convert an enumerator to its corresponding ANSI color-sequence, such that following text is of the specified color (until re-specified). 
        /// @param color The enumerator corresponding to the selected color.
        /// @return The resulting ANSI color-sequence.
        [[nodiscard]] static std::string toANSI(Color color);

        /// @brief Get the color at the top of the stack. If the stack is empty, return the default color.
        /// @return The enumerator corresponding to the current color.
        [[nodiscard]] inline static Color getCurrentColor() { return s_colorStack.empty()? Color::Default: s_colorStack.top(); }
    private:
        static std::stack<Color> s_colorStack;
    };
}