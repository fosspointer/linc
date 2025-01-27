#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/EnumeratorClause.hpp>
#include <linc/tree/NodeListClause.hpp>

namespace linc
{
    class EnumerationDeclaration final : public Declaration
    {
    public:
        EnumerationDeclaration(const Token& enumeration_keyword, const Token& left_brace, const Token& right_brace, std::unique_ptr<const IdentifierExpression> identifier,
            std::unique_ptr<const NodeListClause<EnumeratorClause>> enumerators)
            :Declaration(std::move(identifier), enumeration_keyword.info), m_enumerationKeyword(enumeration_keyword), m_leftBrace(left_brace), m_rightBrace(right_brace),
            m_enumerators(std::move(enumerators))
        {
            addToken(m_enumerationKeyword);
            addTokens(m_identifier->getTokens());
            addToken(m_leftBrace);
            addTokens(m_enumerators->getTokens());
            addToken(m_rightBrace);
        }

        std::unique_ptr<const Declaration> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            auto enumerators = m_enumerators->clone();
            return std::make_unique<const EnumerationDeclaration>(m_enumerationKeyword, m_leftBrace, m_rightBrace, std::move(identifier), std::move(enumerators));
        }

        inline const Token& getEnumerationKeyword() const { return m_enumerationKeyword; }
        inline const Token& getLeftBrace() const { return m_leftBrace; }
        inline const Token& getRightBrace() const { return m_rightBrace; }
        inline const auto* const getEnumerators() const { return m_enumerators.get(); }
    private:
        const Token m_enumerationKeyword, m_leftBrace, m_rightBrace;
        const std::unique_ptr<const NodeListClause<EnumeratorClause>> m_enumerators;
    };
}