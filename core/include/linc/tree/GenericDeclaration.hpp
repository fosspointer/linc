#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class GenericDeclaration final : public Declaration
    {
    public:
        GenericDeclaration(const Token& generic_keyword, const Token& left_angled_bracket, const Token& right_angled_bracket,
            std::unique_ptr<const NodeListClause<IdentifierExpression>> type_identifiers, std::unique_ptr<const Declaration> declaration)
            :Declaration(Types::uniqueCast<const IdentifierExpression>(declaration->getIdentifier()->clone()), generic_keyword.info), m_genericKeyword(generic_keyword),
            m_leftAngledBracket(left_angled_bracket), m_rightAngledBracket(right_angled_bracket),
            m_typeIdentifiers(std::move(type_identifiers)), m_declaration(std::move(declaration))
        {
            addTokens(std::vector<Token>{m_genericKeyword, m_leftAngledBracket});
            addTokens(m_typeIdentifiers->getTokens());
            addToken(m_rightAngledBracket);
            addTokens(m_declaration->getTokens());
        }

        inline const Token& getGenericKeyword() const { return m_genericKeyword; }
        inline const Token& getLeftAngledBracket() const { return m_leftAngledBracket; }
        inline const Token& getRightAngledBracket() const { return m_rightAngledBracket; }
        inline const NodeListClause<IdentifierExpression>* const getTypeIdentifiers() const { return m_typeIdentifiers.get(); }
        inline const Declaration* const getDeclaration() const { return m_declaration.get(); }

        virtual std::unique_ptr<const Declaration> clone() const final override
        {
            return std::make_unique<const GenericDeclaration>(m_genericKeyword, m_leftAngledBracket, m_rightAngledBracket, m_typeIdentifiers->clone(),
                m_declaration->clone());
        }
    private:
        const Token m_genericKeyword, m_leftAngledBracket, m_rightAngledBracket;
        const std::unique_ptr<const NodeListClause<IdentifierExpression>> m_typeIdentifiers;
        const std::unique_ptr<const Declaration> m_declaration;
    };
}