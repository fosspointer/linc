#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ExpressionStatement final : public Statement
    {
    public:
        ExpressionStatement(const Expression* expression)
            :Statement(expression->getInfo()), m_expression(expression)
        {}

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_expression};
        }

        inline const Expression* getExpression() const { return m_expression; }
    private:
        const Expression* m_expression;
    };
}