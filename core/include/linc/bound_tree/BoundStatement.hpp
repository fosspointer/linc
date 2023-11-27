#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundStatement : public BoundNode
    {
    public:
        BoundStatement() = default;
        virtual ~BoundStatement() = default;
    };
}