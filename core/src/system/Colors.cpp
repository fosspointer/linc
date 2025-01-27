#include <linc/system/Colors.hpp>
#include <linc/system/Exception.hpp>

namespace linc
{
    std::stack<Colors::Color> Colors::s_colorStack{};
    bool Colors::s_ansiSupported{true};

    std::string Colors::push(Color color)
    {
        s_colorStack.push(color);
        return toANSI(color);
    }

    std::string Colors::pop()
    {
        if(s_colorStack.empty())
            throw LINC_EXCEPTION_ILLEGAL_STATE(s_colorStack);

        s_colorStack.pop();
        return toANSI(getCurrentColor());
    }

    std::string Colors::toANSI(Color color)
    {
        static const std::string empty{};
        static const std::string default_escape{"\x1B[0m"};
        
        if(!s_ansiSupported)
            return empty;
        else if(color == Colors::Reset)
            return default_escape;
        
        auto base_color = color & 0xF; // bits 0-4 = color
        auto mode = color & 0x30; // bits 5-6 = mode
        bool is_high_intensity = color >> 6; // bit 7 = high intensity
        unsigned short offset;
        if(mode == Colors::Background)
            offset = is_high_intensity? 100: 40;
        else
            offset = is_high_intensity? 90: 30;
        char begin = mode == Colors::Bold? '1': mode == Colors::Underline? '4': '0';
        
        std::string result{"\x1B["};
        result.reserve(7ul);
        result.push_back(begin);
        result.push_back(';');
        result.append(std::to_string(offset + base_color));
        result.push_back('m');
        return result;
    }
}