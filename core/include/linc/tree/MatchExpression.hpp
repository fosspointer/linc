#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/MatchClause.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class MatchExpression final : public Expression
    {
    public:
        MatchExpression(const Token& match_keyword, const Token& left_brace, const Token& right_brace,
            std::unique_ptr<const Expression> test_expression, std::unique_ptr<const NodeListClause<MatchClause>> clauses)
            :Expression(match_keyword.info), m_matchKeyword(match_keyword), m_leftBrace(left_brace), m_rightBrace(right_brace),
            m_testExpression(std::move(test_expression)), m_clauses(std::move(clauses))
        {
            addToken(m_matchKeyword);
            addTokens(m_testExpression->getTokens());
            addToken(m_leftBrace);
            addTokens(m_clauses->getTokens());
            addToken(m_rightBrace);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const MatchExpression>(m_matchKeyword, m_leftBrace, m_rightBrace, m_testExpression->clone(), m_clauses->clone());
        }

        const Token& getMatchKeyword() const { return m_matchKeyword; }
        const Token& getLeftBrace() const { return m_leftBrace; }
        const Token& getRightBrace() const { return m_rightBrace; }
        const Expression* const getTestExpression() const { return m_testExpression.get(); }
        const NodeListClause<MatchClause>* const getClauses() const { return m_clauses.get(); }
    private:
        const Token m_matchKeyword, m_leftBrace, m_rightBrace;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const NodeListClause<MatchClause>> m_clauses;
    };
}