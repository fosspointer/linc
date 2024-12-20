#include <linc/system/Colors.hpp>
#include <linc/system/Exception.hpp>

namespace linc
{
    std::stack<Colors::Color> Colors::s_colorStack{};

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
        static const std::string black  = "\x1B[0;30m";
        static const std::string red    = "\x1B[0;31m";
        static const std::string green  = "\x1B[0;32m";
        static const std::string yellow = "\x1B[0;33m";
        static const std::string blue   = "\x1B[0;34m";
        static const std::string purple = "\x1B[0;35m";
        static const std::string cyan   = "\x1B[0;36m";
        static const std::string white  = "\x1B[0;37m";
        static const std::string _default  = "\x1B[0m";

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
        default: return _default;
        }
    }
}