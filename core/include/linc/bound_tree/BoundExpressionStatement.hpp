#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundExpressionStatement final : public BoundStatement
    {
    public:
        BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression)
            :BoundStatement(expression->getType()), m_expression(std::move(expression))
        {}

        inline const BoundExpression* const getExpression() const { return m_expression.get(); } 
    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Expression Statement";
        }
        
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}