#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundExpressionStatement final : public BoundStatement
    {
    public:
        BoundExpressionStatement(const BoundExpression* expression)
            :m_expression(expression)
        {}

        inline const BoundExpression* getExpression() const { return m_expression; } 
    private:
        const BoundExpression* m_expression;
    };
}