#pragma once
#include <cstdint>
#include <stack>
#include <string>

namespace linc
{
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

        static std::string push(Color color);
        static std::string pop();
        [[nodiscard]] static std::string toANSI(Color color);
        [[nodiscard]] inline static Color getCurrentColor() { return s_colorStack.empty()? Color::Reset: s_colorStack.top(); }
        static inline void toggleANSISupport(bool enabled) { s_ansiSupported = enabled; }
    private:
        static std::stack<Color> s_colorStack;
        static bool s_ansiSupported; 
    };

    inline Colors::Color operator|(Colors::Color first, Colors::Color second)
    {
        return static_cast<Colors::Color>(+first | +second);
    }
}
