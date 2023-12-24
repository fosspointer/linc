#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ExpressionStatement final : public Statement
    {
    public:
        ExpressionStatement(std::unique_ptr<const Expression> expression)
            :Statement(expression->getInfo()), m_expression(std::move(expression))
        {}

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_expression.get()};
        }

        inline const Expression* getExpression() const { return m_expression.get(); }
        virtual std::unique_ptr<const Statement> clone_const() const final override
        {
            return std::make_unique<const ExpressionStatement>(std::move(m_expression->clone_const()));
        }
    private:
        const std::unique_ptr<const Expression> m_expression;
    };
}