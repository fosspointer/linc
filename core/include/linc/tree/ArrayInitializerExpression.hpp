#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/NodeListClause.hpp>

namespace linc
{
    class ArrayInitializerExpression final : public Expression 
    {
    public:
        ArrayInitializerExpression(const Token& left_bracket, const Token& right_bracket, std::unique_ptr<const NodeListClause<Expression>> values)
            :Expression(left_bracket.info), m_leftBracket(left_bracket), m_rightBracket(right_bracket), m_values(std::move(values))
        {
            addToken(m_leftBracket);
            addTokens(m_values->getTokens());
            addToken(m_rightBracket);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const ArrayInitializerExpression>(m_leftBracket, m_rightBracket, m_values->clone());
        }
        
        inline const Token& getLeftBracket() const { return m_leftBracket; }
        inline const Token& getRightBracket() const { return m_rightBracket; }
        inline const NodeListClause<Expression>* const getValues() const { return m_values.get(); }
    private:
        const Token m_leftBracket, m_rightBracket;
        const std::unique_ptr<const NodeListClause<Expression>> m_values;
    };
}