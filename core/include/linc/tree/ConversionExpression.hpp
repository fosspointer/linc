#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class ConversionExpression final : public Expression 
    {
    public:
        ConversionExpression(const Token& as_keyword, const Token& left_parenthesis, const Token& right_parenthesis,
            std::unique_ptr<const TypeExpression> type, std::unique_ptr<const Expression> expression)
            :Expression(as_keyword.info), m_asKeyword(as_keyword), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
            m_type(std::move(type)), m_expression(std::move(expression))
        {
            addTokens(std::vector<Token>{m_asKeyword, m_leftParenthesis});
            addTokens(m_expression->getTokens());
            addToken(m_rightParenthesis);
        }

        inline const Token& getAsKeyword() const { return m_asKeyword; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const TypeExpression* const getType() const { return m_type.get(); }
        inline const Expression* const getExpression() const { return m_expression.get(); }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            auto type = Types::uniqueCast<const TypeExpression>(m_type->clone());
            
            return std::make_unique<const ConversionExpression>(m_asKeyword, m_leftParenthesis, m_rightParenthesis, std::move(type),
                m_expression->clone()); 
        }
    private:
        const Token m_asKeyword, m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const TypeExpression> m_type;
        const std::unique_ptr<const Expression> m_expression;
    };
}