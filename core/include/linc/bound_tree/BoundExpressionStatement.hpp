#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundExpressionStatement final : public BoundStatement
    {
    public:
        BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression)
            :m_expression(std::move(expression))
        {}

        inline const BoundExpression* getExpression() const { return m_expression.get(); } 
    private:
        std::unique_ptr<const BoundExpression> m_expression;
    };
}