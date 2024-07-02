#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ArrayInitializerExpression final : public Expression 
    {
    public:
        struct Argument final
        {
            const std::optional<Token> separator;
            std::unique_ptr<const Expression> value;
        };

        ArrayInitializerExpression(const Token& left_bracket, const Token& right_bracket, std::vector<Argument> values)
            :Expression(left_bracket.info), m_leftBracket(left_bracket), m_rightBracket(right_bracket), m_values(std::move(values))
        {
            addToken(m_leftBracket);

            for(const auto& value: m_values)
            {
                addTokens(value.value->getTokens());
                if(value.separator)
                    addToken(*value.separator);
            }

            addToken(m_rightBracket);
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<Argument> values;

            for(const auto& value: m_values)
                values.push_back(Argument{.separator = value.separator, .value = value.value->clone()});

            return std::make_unique<const ArrayInitializerExpression>(m_leftBracket, m_rightBracket, std::move(values));
        }
        
        inline const Token& getLeftBracket() const { return m_leftBracket; }
        inline const Token& getRightBracket() const { return m_rightBracket; }
        inline const std::vector<Argument>& getValues() const { return m_values; }
    private:
        const Token m_leftBracket, m_rightBracket;
        const std::vector<Argument> m_values;
    };
}