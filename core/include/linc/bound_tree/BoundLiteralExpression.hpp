#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundLiteralExpression final : public BoundExpression
    {
    public:
        using Type = Types::Type;
        inline TypedValue getValue() const { return m_value; }
        
        BoundLiteralExpression(const TypedValue& value)
            :BoundExpression(value.getType()), m_value(value)
        {}
    private:
        const TypedValue m_value;
    };
}