#pragma once
#include <linc/system/Value.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct BreakException final { std::string label; };
    struct ContinueException final { std::string label; };
    struct ReturnException final { class Value returnValue; };
}