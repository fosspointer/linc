#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct Program final
    {
        std::vector<std::unique_ptr<const Declaration>> declarations{};
    };
}