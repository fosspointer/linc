#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class RangeExpression final : public Expression
    {
    public:
        RangeExpression(const Token& range_specifier, std::unique_ptr<const Expression> begin_expression, std::unique_ptr<const Expression> end_expression)
            :Expression(range_specifier.info), m_rangeSpecifier(range_specifier), m_beginExpression(std::move(begin_expression)),
            m_endExpression(std::move(end_expression))
        {
            addTokens(m_beginExpression->getTokens());
            addToken(m_rangeSpecifier);
            addTokens(m_endExpression->getTokens());
        }

        inline const Token& getRangeSpecifier() const { return m_rangeSpecifier; }
        inline const Expression* const getBeginExpression() const { return m_beginExpression.get(); }
        inline const Expression* const getEndExpression() const { return m_endExpression.get(); }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const RangeExpression>(m_rangeSpecifier, m_beginExpression->clone(), m_endExpression->clone());
        }
    private:
        const Token m_rangeSpecifier;
        const std::unique_ptr<const Expression> m_beginExpression, m_endExpression;
    };
}