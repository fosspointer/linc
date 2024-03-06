#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ArrayInitializerExpression final : public Expression 
    {
    public:
        ArrayInitializerExpression(const Token& left_bracket, const Token& right_bracket, std::vector<std::unique_ptr<const Expression>> values)
            :m_leftBracket(left_bracket), m_rightBracket(right_bracket), m_values(std::move(values))
        {
            addToken(m_leftBracket);

            for(const auto& value: m_values)
                addTokens(value->getTokens());

            addToken(m_rightBracket);
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            std::vector<std::unique_ptr<const Expression>> values;

            for(const auto& value: m_values)
                values.push_back(value->cloneConst());

            return std::make_unique<const ArrayInitializerExpression>(m_leftBracket, m_rightBracket, std::move(values));
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            std::vector<const Node*> children;

            for(const auto& value: m_values)
                children.push_back(value.get());
            
            return children;
        }

        inline const Token& getLeftBracket() const { return m_leftBracket; }
        inline const Token& getRightBracket() const { return m_rightBracket; }
        inline const std::vector<std::unique_ptr<const Expression>>& getValues() const { return m_values; }
    private:
        const Token m_leftBracket, m_rightBracket;
        const std::vector<std::unique_ptr<const Expression>> m_values;
    };
}