#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class ShellExpression final : public Expression
    {
    public:
        ShellExpression(const Token& identifier_token, const Token& left_parenthesis_token, const Token& right_parenthesis_token,
            std::unique_ptr<const Expression> expression)
            :Expression(identifier_token.info), m_identifierToken(identifier_token), m_leftParenthesisToken(left_parenthesis_token),
            m_rightParenthesisToken(right_parenthesis_token), m_expression(std::move(expression))
        {
            addTokens(std::vector<Token>{m_identifierToken, m_leftParenthesisToken});
            addTokens(m_expression->getTokens());
            addToken(m_rightParenthesisToken);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const ShellExpression>(m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken, m_expression->clone());
        }

        const Expression* getExpression() const { return m_expression.get(); }
        const Token& getIdentifierToken() const { return m_identifierToken; }
        const Token& getLeftParenthesis() const { return m_leftParenthesisToken; }
        const Token& getRightParenthesis() const { return m_rightParenthesisToken; }
    private:
        const Token m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken;
        const std::unique_ptr<const Expression> m_expression;
    };
}