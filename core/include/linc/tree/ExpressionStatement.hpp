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
    private:
        std::unique_ptr<const Expression> m_expression;
    };
}