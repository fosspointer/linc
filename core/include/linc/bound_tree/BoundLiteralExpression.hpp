#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundLiteralExpression final : public BoundExpression
    {
    public:
        BoundLiteralExpression(const PrimitiveValue& value, const Types::type& type);
        [[nodiscard]] PrimitiveValue getValue() const;

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const PrimitiveValue m_value;
    };
}