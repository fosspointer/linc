#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class PutCharacterStatement final : public Statement
    {
    public:
        PutCharacterStatement(const Token& identifier_token, const Token& left_parenthesis_token, 
        const Token& right_parenthesis_token, std::unique_ptr<const Expression> expression)
            :Statement(NodeInfo{.info = left_parenthesis_token.info}), 
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

        virtual std::unique_ptr<const Statement> cloneConst() const final override
        {
            return std::make_unique<const PutCharacterStatement>(m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken,
                std::move(m_expression->cloneConst()));
        }

        const Expression* getExpression() const { return m_expression.get(); }
        const Token& getIdentifierToken() const { return m_leftParenthesisToken; }
        const Token& getLeftParenthesis() const { return m_leftParenthesisToken; }
        const Token& getRightParenthesis() const { return m_rightParenthesisToken; }
    private:
        const Token m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken;
        const std::unique_ptr<const Expression> m_expression;
    };
}