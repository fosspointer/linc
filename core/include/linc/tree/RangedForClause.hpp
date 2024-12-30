#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Clause.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class RangedForClause final : public Clause<RangedForClause>
    {
    public:
        RangedForClause(const Token& in_keyword, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const Expression> expression)
            :Clause<RangedForClause>(in_keyword.info), m_inKeyword(in_keyword), m_identifier(std::move(identifier)), m_expression(std::move(expression))
        {
            addTokens(m_identifier->getTokens());
            addToken(m_inKeyword);
            addTokens(m_expression->getTokens());
        }

        inline const Token& getInKeyword() const { return m_inKeyword; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const Expression* const getExpression() const { return m_expression.get(); }

        std::unique_ptr<const RangedForClause> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const RangedForClause>(m_inKeyword, std::move(identifier), m_expression->clone());
        }
    private:
        const Token m_inKeyword;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const Expression> m_expression;
    };
}