#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/system/Types.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class DirectVariableDeclaration final : public Declaration
    {
    public:
        DirectVariableDeclaration(const Token& direct_assignment, const std::optional<Token>& mutability_specifier, 
            std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const Expression> value)
            :Declaration(direct_assignment.info), m_directAssignment(direct_assignment), m_mutabilitySpecifier(mutability_specifier), 
            m_identifier(std::move(identifier)), m_value(std::move(value))
        {
            addTokens(m_identifier->getTokens());
            
            if(mutability_specifier)
                addToken(*mutability_specifier);

            addToken(direct_assignment);
            addTokens(m_value->getTokens());
        }

        virtual std::unique_ptr<const Declaration> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const DirectVariableDeclaration>(m_directAssignment, m_mutabilitySpecifier, std::move(identifier), m_value->clone());
        }

        inline const Token& getDirectAssignment() const { return m_directAssignment; }
        inline const std::optional<Token>& getMutabilitySpecifier() const { return m_mutabilitySpecifier; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const Expression* const getValue() const { return m_value.get(); }
    private:
        const Token m_directAssignment;
        const std::optional<Token> m_mutabilitySpecifier;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const Expression> m_value;
    };
}