#pragma once
#include <linc/tree/Clause.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/TypeExpression.hpp>
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    class EnumeratorClause final : public Clause<EnumeratorClause>
    {
    public:
        EnumeratorClause(const Token& left_parenthesis, const Token& right_parenthesis, std::unique_ptr<const IdentifierExpression> identifier, 
            std::unique_ptr<const TypeExpression> actual_type)
            :Clause(identifier->getTokenInfo()), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis), m_identifier(std::move(identifier)),
            m_actualType(std::move(actual_type))
        {
            addTokens(m_identifier->getTokens());
            addToken(m_leftParenthesis);
            addTokens(m_actualType->getTokens());
            addToken(m_rightParenthesis);
        }

        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const TypeExpression* const getActualType() const { return m_actualType.get(); }

        virtual std::unique_ptr<const EnumeratorClause> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            auto actual_type = Types::uniqueCast<const TypeExpression>(m_actualType->clone());
            return std::make_unique<const EnumeratorClause>(m_leftParenthesis, m_rightParenthesis, std::move(identifier), std::move(actual_type));
        }
    private:
        const Token m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const TypeExpression> m_actualType;
    };
}