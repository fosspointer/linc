#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/Colors.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct TextSpan final
    {
        std::string text;
        std::string::size_type spanStart{};
        std::string::size_type spanEnd{};

        std::string get(Colors::Color color = Colors::Color::Red) const
        {
            return linc::Logger::format("$:$:$:$:$", text.substr(0, spanStart), Colors::toANSI(color),
                text.substr(spanStart, spanEnd - spanStart), Colors::toANSI(Colors::Color::Default), text.substr(spanEnd));
        }
    };
}