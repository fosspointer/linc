#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ParenthesisExpression final : public Expression
    {
    public:
        ParenthesisExpression(const Token& left_parenthesis, const Token& right_parenthesis, std::unique_ptr<const Expression> expression)
            :Expression(NodeInfo{
                .tokenList = {left_parenthesis}, .info = left_parenthesis.info}),
            m_expression(std::move(expression)), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis)
        {
            LINC_NODE_ASSERT(m_leftParenthesis, Token::Type::ParenthesisLeft);
            LINC_NODE_ASSERT(m_rightParenthesis, Token::Type::ParenthesisRight);
            addTokens(m_expression->getTokens());
            addToken(m_rightParenthesis);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const ParenthesisExpression>(m_leftParenthesis, m_rightParenthesis, m_expression->clone());
        }

        inline const Expression* const getExpression() const { return m_expression.get(); }
        inline const Expression* const getExpression() { return m_expression.get(); }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
    private:
        const std::unique_ptr<const Expression> m_expression;
        const Token m_leftParenthesis, m_rightParenthesis;
    };
}