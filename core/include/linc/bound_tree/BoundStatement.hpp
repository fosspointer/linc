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
        virtual std::unique_ptr<const BoundStatement> clone_const() const = 0;
    };
}