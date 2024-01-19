#pragma once
#include <linc/system/Logger.hpp>
#include <linc/Include.hpp>

namespace linc
{
    enum class Color: char
    {
        Default, Black, Red, Green, Yellow, Blue, Purple, Cyan, White
    };

    struct TextSpan
    {
        std::string text;
        std::string::size_type spanStart{};
        std::string::size_type spanEnd{};

        std::string get(Color color = Color::Red) const
        {
            return linc::Logger::format("$:$:$:$:$:", text.substr(0, spanStart), GetColorSequence(color),
                text.substr(spanStart, spanEnd), GetColorSequence(Color::Default), text.substr(spanEnd));
        }
    private:
        static std::string GetColorSequence(Color color)
        {
            const std::string black  = "\e[0;30m";
            const std::string red    = "\e[0;31m";
            const std::string green  = "\e[0;32m";
            const std::string yellow = "\e[0;33m";
            const std::string blue   = "\e[0;34m";
            const std::string purple = "\e[0;35m";
            const std::string cyan   = "\e[0;36m";
            const std::string white  = "\e[0;37m";
            const std::string _default  = "\e[0m";

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