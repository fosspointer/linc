#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ExpressionStatement final : public Statement
    {
    public:
        ExpressionStatement(const Token& terminator, std::unique_ptr<const Expression> expression)
            :Statement(terminator, expression->getInfo()), m_expression(std::move(expression))
        {}

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            return std::make_unique<const ExpressionStatement>(getTerminator(), m_expression->clone());
        }

        inline const Expression* getExpression() const { return m_expression.get(); }
    private:
        const std::unique_ptr<const Expression> m_expression;
    };
}