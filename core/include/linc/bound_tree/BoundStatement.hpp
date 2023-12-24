#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundStatement : public BoundNode
    {
    public:
        BoundStatement(Types::Type type)
            :m_type(type)
        {}

        virtual ~BoundStatement() = default;

        Types::Type getType() const { return m_type; }
    private:
        const Types::Type m_type;
    };
}