#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundLiteralExpression final : public BoundExpression
    {
    public:
        BoundLiteralExpression(const TypedValue& value);
        [[nodiscard]] TypedValue getValue() const;

        virtual std::unique_ptr<const BoundExpression> clone_const() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const TypedValue m_value;
    };
}