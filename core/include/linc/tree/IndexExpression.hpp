#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class IndexExpression final : public Expression
    {
    public:
        IndexExpression(const Token& left_bracket, const Token& right_bracket, std::unique_ptr<const Expression> array,
            std::unique_ptr<const Expression> index)
            :Expression(array->getTokenInfo()), m_leftBracket(left_bracket), m_rightBracket(right_bracket), m_array(std::move(array)),
            m_index(std::move(index))
        {
            addTokens(m_array->getTokens());
            addToken(m_leftBracket);
            addTokens(m_index->getTokens());
            addToken(m_rightBracket);
        }

        inline const Token& getLeftBracket() const { return m_leftBracket; }
        inline const Token& getRightBracket() const { return m_rightBracket; }
        inline const Expression* const getArray() const { return m_array.get(); }
        inline const Expression* const getIndex() const { return m_index.get(); }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const IndexExpression>(m_leftBracket, m_rightBracket, m_array->clone(), m_index->clone());
        }
    private:
        const Token m_leftBracket, m_rightBracket;
        const std::unique_ptr<const Expression> m_array, m_index;
    };
}