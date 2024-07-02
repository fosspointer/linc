#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class JumpStatement final : public Statement
    {
    public:
        JumpStatement(const Token& jump_keyword, std::unique_ptr<const IdentifierExpression> identifier)
            :Statement(jump_keyword.info), m_jumpKeyword(jump_keyword), m_identifier(std::move(identifier))
        {
            addToken(m_jumpKeyword);
            addTokens(m_identifier->getTokens());
        }

        inline const Token& getJumpKeyword() const { return m_jumpKeyword; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const JumpStatement>(m_jumpKeyword, std::move(identifier));
        } 
    private:
        const Token m_jumpKeyword;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
    };
}