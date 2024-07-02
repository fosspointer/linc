#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class TypeExpression final : public Expression
    {
    public:
        struct ArraySpecifier final
        {
            std::optional<const Token> leftBracket, rightBracket;
            std::unique_ptr<const LiteralExpression> count;
        };

        TypeExpression(const Token& type_identifier, const std::optional<Token>& mutability_Keyword, std::vector<ArraySpecifier> array_specifiers)
            :Expression(type_identifier.info), m_typeIdentifier(type_identifier), m_mutabilityKeyword(mutability_Keyword),
            m_arraySpecifiers(std::move(array_specifiers))
        {
            handleTokens();
        }

        TypeExpression(const Token& type_identifier, const std::optional<Token>& mutability_Keyword)
            :Expression(type_identifier.info), m_typeIdentifier(type_identifier), m_mutabilityKeyword(mutability_Keyword), m_arraySpecifiers{}
        {
            handleTokens();
        }

        inline const Token& getTypeIdentifier() const { return m_typeIdentifier; }
        inline const std::optional<const Token>& getMutabilityKeyword() const { return m_mutabilityKeyword; }
        inline const std::vector<ArraySpecifier>& getArraySpecifiers() const { return m_arraySpecifiers; }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<ArraySpecifier> array_specifiers;
            array_specifiers.reserve(m_arraySpecifiers.size());

            for(const auto& specifier: m_arraySpecifiers)
            {
                auto count = specifier.count? specifier.count->clone(): nullptr;
                array_specifiers.push_back(ArraySpecifier{
                    .leftBracket = specifier.leftBracket, .rightBracket = specifier.rightBracket,
                    .count = Types::unique_cast<const LiteralExpression>(std::move(count))
                });
            }

            return std::make_unique<const TypeExpression>(m_typeIdentifier, m_mutabilityKeyword, std::move(array_specifiers));
        }
    private:
        inline void handleTokens() const
        {
            if(m_mutabilityKeyword)
                addToken(*m_mutabilityKeyword);
            
            addToken(m_typeIdentifier);
            
            for(const auto& specifier: m_arraySpecifiers)
                if(specifier.leftBracket && specifier.rightBracket)
                {
                    addToken(*specifier.leftBracket);
                    if(specifier.count)
                        addTokens(specifier.count->getTokens());
                    addToken(*specifier.rightBracket);
                }
        }

        const Token m_typeIdentifier;
        const std::optional<const Token> m_mutabilityKeyword;
        const std::vector<ArraySpecifier> m_arraySpecifiers;
    };
}