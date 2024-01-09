#pragma once
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class VariableAssignmentExpression final : public Expression
    {
    public:
        VariableAssignmentExpression(const Token& equals_token, const Token& identifier_token, std::unique_ptr<const Expression> value)
            :m_equalsToken(equals_token), m_identifierToken(identifier_token), m_value(std::move(value))
        {
            addToken(m_identifierToken);
            addToken(m_equalsToken);
            addTokens(m_value->getTokens());
        }

        const Token& getEqualsToken() const { return m_equalsToken; }
        const Token& getIdentifierToken() const { return m_identifierToken; }
        const Expression* const getValue() const { return m_value.get(); }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_value.get()};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const VariableAssignmentExpression>(m_equalsToken, m_identifierToken, std::move(m_value->clone_const()));
        }
    private:
        const Token m_equalsToken, m_identifierToken;
        const std::unique_ptr<const Expression> m_value;
    };
}