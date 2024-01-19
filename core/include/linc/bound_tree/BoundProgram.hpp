#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct BoundProgram final
    {
        std::vector<std::unique_ptr<const BoundDeclaration>> declarations;
    };
}