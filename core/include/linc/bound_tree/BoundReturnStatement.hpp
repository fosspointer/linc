#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundReturnStatement final : public BoundStatement
    {
    public:
        BoundReturnStatement(std::unique_ptr<const BoundExpression> expression)
            :BoundStatement(expression->getType()), m_expression(std::move(expression))
        {}

        [[nodiscard]] const BoundExpression* const getExpression() const { return m_expression.get(); }

        virtual std::unique_ptr<const BoundStatement> clone() const final override
        {
            return std::make_unique<const BoundReturnStatement>(m_expression->clone());
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Return Statement";
        }
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}