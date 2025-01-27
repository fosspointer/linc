#pragma once
#include <linc/Include.hpp>
#define LINC_COLORS_EXP(number) 1 << number

namespace linc
{
    /// @brief Color utility static class
    class Colors final
    {
    public:
        Colors() = delete;

        /// @brief Enum representing ANSI colors, exclusive options are incremented by one whereas non-exclusive options
        /// are assigned to the next power of two, so as to allow bit manipulation.
        enum Color: std::uint_least8_t
        {
            // Color hue
            Black = 0, Red = 1, Green = 2, Yellow = 3, Blue = 4, Purple = 5, Cyan = 6, White = 7,
            // Reset color
            Reset = 8,
            // Mode
            Bold = 16, Underline = 32, Background = 48,
            // High Intensity
            HighIntensity = 64,
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
        [[nodiscard]] inline static Color getCurrentColor() { return s_colorStack.empty()? Color::Reset: s_colorStack.top(); }

        /// @brief Toggle the support for ANSI escape sequences. If disabled, toANSI will return an empty string.
        static inline void toggleANSISupport(bool option) { s_ansiSupported = option; }
    private:
        static std::stack<Color> s_colorStack;
        static bool s_ansiSupported;
    };

    inline Colors::Color operator|(Colors::Color first, Colors::Color second)
    {
        return static_cast<Colors::Color>(+first | +second);
    }
}