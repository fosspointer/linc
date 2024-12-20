#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Clause.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/NodeListClause.hpp>

namespace linc
{
    class MatchClause final : public Clause<MatchClause>
    {
    public:
        MatchClause(const Token& then_keyword, std::unique_ptr<const Expression> expression, std::unique_ptr<const NodeListClause<Expression>> values)
            :Clause(then_keyword.info), m_thenKeyword(then_keyword), m_expression(std::move(expression)), m_values(std::move(values))
        {
            addTokens(m_values->getTokens());
            addToken(m_thenKeyword);
            addTokens(m_expression->getTokens());
        }

        inline const Token& getThenKeyword() const { return m_thenKeyword; }
        inline const Expression* const getExpression() const { return m_expression.get(); }
        inline const auto* const getValues() const { return m_values.get(); }

        std::unique_ptr<const MatchClause> clone() const final override
        {
            return std::make_unique<const MatchClause>(m_thenKeyword, m_expression->clone(), m_values->clone());
        }
    private:
        const Token m_thenKeyword;
        const std::unique_ptr<const Expression> m_expression;
        const std::unique_ptr<const NodeListClause<Expression>> m_values;
    };
}