#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundStatement : public BoundNode
    {
    public:
        BoundStatement(Types::Type type)
            :BoundNode(type)
        {}

        virtual ~BoundStatement() = default;
    };
}