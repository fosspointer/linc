#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/NodeListClause.hpp>

namespace linc
{
    class CallExpression final : public Expression
    {
    public:
        CallExpression(const Token& left_parenthesis, const Token& right_parenthesis, std::unique_ptr<const Expression> function,
            std::unique_ptr<const NodeListClause<Expression>> arguments, bool is_external)
            :Expression(function->getTokenInfo()), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
            m_function(std::move(function)), m_arguments(std::move(arguments)), m_isExternal(is_external)
        {
            addTokens(m_function->getTokens());
            addToken(m_leftParenthesis);
            addTokens(m_arguments->getTokens());
            addToken(m_rightParenthesis);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const CallExpression>(m_leftParenthesis, m_rightParenthesis,m_function->clone(), m_arguments->clone(), m_isExternal);
        }

        const Expression* const getFunction() const { return m_function.get(); }
        const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        const Token& getRightParenthesis() const { return m_rightParenthesis; }
        const auto* const getArguments() const { return m_arguments.get(); }
        bool isExternal() const { return m_isExternal; }
    private:
        const Token m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const Expression> m_function;
        const std::unique_ptr<const NodeListClause<Expression>> m_arguments;
        const bool m_isExternal;
    };
}