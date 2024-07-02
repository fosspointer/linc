#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundExpression : public BoundNode
    {
    public:
        BoundExpression(const Types::type& type)
            :BoundNode(type)
        {}
        
        virtual ~BoundExpression() = default;
        virtual std::unique_ptr<const BoundExpression> clone() const = 0;
    };
}