#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundLiteralExpression final : public BoundExpression
    {
    public:
        inline TypedValue getValue() const { return m_value; }
        
        BoundLiteralExpression(const TypedValue& value)
            :BoundExpression(value.getType()), m_value(value)
        {}

    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Literal Expression";
        }
        const TypedValue m_value;
    };
}