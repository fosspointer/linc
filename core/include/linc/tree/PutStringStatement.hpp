#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class PutStringStatement final : public Statement
    {
    public:
        PutStringStatement(const Token& identifier_token, const Token& left_parenthesis_token, 
        const Token& right_parenthesis_token, std::unique_ptr<const Expression> expression)
            :Statement(NodeInfo{.lineNumber = left_parenthesis_token.lineNumber}), 
            m_identifierToken(identifier_token), m_leftParenthesisToken(left_parenthesis_token),
            m_rightParenthesisToken(right_parenthesis_token), m_expression(std::move(expression))
        {
            addToken(m_identifierToken);
            addToken(m_leftParenthesisToken);
            addTokens(m_expression->getTokens());
            addToken(m_rightParenthesisToken);
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_expression.get()};
        }

        virtual std::unique_ptr<const Statement> clone_const() const final override
        {
            return std::make_unique<const PutStringStatement>(m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken,
                std::move(m_expression->clone_const()));
        }

        const Expression* getExpression() const { return m_expression.get(); }
        const Token& getIdentifierToken() const { return m_leftParenthesisToken; }
        const Token& getLeftParenthesisToken() const { return m_leftParenthesisToken; }
        const Token& getRightParenthesisToken() const { return m_rightParenthesisToken; }
    private:
        const Token m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken;
        const std::unique_ptr<const Expression> m_expression;
    };
}