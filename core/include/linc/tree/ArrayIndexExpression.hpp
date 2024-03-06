#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class ArrayIndexExpression final : public Expression
    {
    public:
        ArrayIndexExpression(const Token& left_bracket, const Token& right_bracket, std::unique_ptr<const IdentifierExpression> identifier,
            std::unique_ptr<const Expression> index)
            :m_leftBracket(left_bracket), m_rightBracket(right_bracket), m_identifier(std::move(identifier)), m_index(std::move(index))
        {
            addTokens(m_identifier->getTokens());
            addToken(m_leftBracket);
            addTokens(m_index->getTokens());
            addToken(m_rightBracket);
        }

        inline const Token& getLeftBracket() const { return m_leftBracket; }
        inline const Token& getRightBracket() const { return m_rightBracket; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const Expression* const getIndex() const { return m_index.get(); }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_identifier.get(), m_index.get()};
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->cloneConst());
            return std::make_unique<const ArrayIndexExpression>(m_leftBracket, m_rightBracket, std::move(identifier), m_index->cloneConst());
        }
    private:
        const Token m_leftBracket, m_rightBracket;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const Expression> m_index;
    };
}