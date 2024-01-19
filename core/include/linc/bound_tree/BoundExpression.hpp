#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundExpression : public BoundNode
    {
    public:
        BoundExpression(Types::Type type)
            :BoundNode(type)
        {}
        
        virtual ~BoundExpression() = default;
        virtual std::unique_ptr<const BoundExpression> clone_const() const = 0;
    };
}