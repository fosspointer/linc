#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundStatement : public BoundNode
    {
    public:
        virtual ~BoundStatement() = default;
        virtual std::unique_ptr<const BoundStatement> clone() const = 0;
    };
}