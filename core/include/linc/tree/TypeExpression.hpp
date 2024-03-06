#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class TypeExpression final : public Expression
    {
    public:
        TypeExpression(const Token& type_identifier, const std::optional<Token>& mutability_Keyword, const std::optional<Token>& left_bracket,
            const std::optional<Token>& right_bracket, std::unique_ptr<const LiteralExpression> array_size)
            :m_typeIdentifier(type_identifier), m_mutabilityKeyword(mutability_Keyword), m_leftBracket(left_bracket),
            m_rightBracket(right_bracket), m_arraySize(std::move(array_size))
        {}

        TypeExpression(const Token& type_identifier, const std::optional<Token>& mutability_Keyword, const std::optional<Token>& left_bracket,
            const std::optional<Token>& right_bracket)
            :m_typeIdentifier(type_identifier), m_mutabilityKeyword(mutability_Keyword), m_leftBracket(left_bracket),
            m_rightBracket(right_bracket), m_arraySize(std::nullopt)
        {}

        inline const Token& getTypeIdentifier() const { return m_typeIdentifier; }
        inline const std::optional<const Token>& getMutabilityKeyword() const { return m_mutabilityKeyword; }
        inline const std::optional<const Token>& getLeftBracket() const { return m_leftBracket; }
        inline const std::optional<const Token>& getRightBracket() const { return m_rightBracket; }
        inline std::optional<const LiteralExpression* const> getArraySize() const
        {
            return m_arraySize? std::make_optional((*m_arraySize).get()): std::nullopt;
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {};
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            if(m_arraySize)
            {
                auto array_size = Types::unique_cast<const LiteralExpression>((*m_arraySize)->cloneConst());

                return std::make_unique<const TypeExpression>(m_typeIdentifier, m_mutabilityKeyword, m_leftBracket, m_rightBracket, std::move(array_size));
            }

            return std::make_unique<const TypeExpression>(m_typeIdentifier, m_mutabilityKeyword, m_leftBracket, m_rightBracket);
        }
    private:
        const Token m_typeIdentifier;
        const std::optional<const Token> m_mutabilityKeyword;
        const std::optional<const Token> m_leftBracket, m_rightBracket;
        const std::optional<const std::unique_ptr<const LiteralExpression>> m_arraySize;
    };
}