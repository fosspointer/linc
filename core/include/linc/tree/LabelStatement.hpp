#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class LabelStatement final : public Statement
    {
    public:
        LabelStatement(const Token& label_specifier, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const Statement> next)
            :Statement(label_specifier.info), m_labelSpecifier(label_specifier), m_identifier(std::move(identifier)), m_next(std::move(next))
        {
            addTokens(m_identifier->getTokens());
            addToken(m_labelSpecifier);
        }

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const LabelStatement>(m_labelSpecifier, std::move(identifier), m_next->clone());
        }

        inline const Token& getLabelSpecifier() const { return m_labelSpecifier; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const Statement* const getNext() const { return m_next.get(); }
    private:
        const Token m_labelSpecifier;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const Statement> m_next;
    };
}