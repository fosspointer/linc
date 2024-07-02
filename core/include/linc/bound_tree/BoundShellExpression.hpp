#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundShellExpression final : public BoundExpression
    {
    public:
        BoundShellExpression(std::unique_ptr<const BoundExpression> expression)
            :BoundExpression(Types::fromKind(Types::Kind::string)), m_expression(std::move(expression))
        {}

        [[nodiscard]] const BoundExpression* const getExpression() const { return m_expression.get(); }

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            return std::make_unique<const BoundShellExpression>(m_expression->clone());
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Read Shell Expression";
        }

        const std::unique_ptr<const BoundExpression> m_expression;
    };
}