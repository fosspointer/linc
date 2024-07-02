#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundStatement : public BoundNode
    {
    public:
        BoundStatement(const Types::type& type)
            :BoundNode(type)
        {}

        virtual ~BoundStatement() = default;
        virtual std::unique_ptr<const BoundStatement> clone() const = 0;
    };
}