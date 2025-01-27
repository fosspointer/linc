#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class AliasDeclaration final : public Declaration
    {
    public:
        AliasDeclaration(const Token& alias_keyword, const Token& assignment_specifier, std::unique_ptr<const IdentifierExpression> identifier,
            std::unique_ptr<const TypeExpression> type)
            :Declaration(std::move(identifier), alias_keyword.info), m_aliasKeyword(alias_keyword), m_assignmentSpecifier(assignment_specifier), m_type(std::move(type))
        {
            addToken(m_aliasKeyword);
            addTokens(m_identifier->getTokens());
            addToken(m_assignmentSpecifier);
            addTokens(m_type->getTokens());
        }

        inline const Token& getAliasKeyword() const { return m_aliasKeyword; }
        inline const Token& getAssignmentSpecifier() const { return m_assignmentSpecifier; }
        inline const TypeExpression* const getType() const { return m_type.get(); }

        virtual std::unique_ptr<const Declaration> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            auto type = Types::uniqueCast<const TypeExpression>(m_type->clone());
            return std::make_unique<const AliasDeclaration>(m_aliasKeyword, m_assignmentSpecifier, std::move(identifier), std::move(type));
        }
    private:
        const Token m_aliasKeyword, m_assignmentSpecifier;
        const std::unique_ptr<const TypeExpression> m_type;
    };
}