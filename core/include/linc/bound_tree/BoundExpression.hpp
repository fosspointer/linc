#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundExpression : public BoundNode
    {
    public:
        BoundExpression(Types::Type type)
            :m_type(type)
        {}
        
        virtual ~BoundExpression() = default;

        Types::Type getType() const { return m_type; }
    private:
        const Types::Type m_type;
    };
}