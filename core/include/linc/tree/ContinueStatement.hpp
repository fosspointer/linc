#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class ContinueStatement final : public Statement
    {
    public:
        ContinueStatement(const Token& terminator, const Token& continue_keyword, std::unique_ptr<const IdentifierExpression> label)
            :Statement(terminator, continue_keyword.info), m_continueKeyword(continue_keyword), m_label(std::move(label))
        {
            addToken(m_continueKeyword);
            
            if(m_label)
                addTokens(m_label->getTokens());
        }

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            auto label = m_label? Types::uniqueCast<const IdentifierExpression>(m_label->clone()): nullptr;
            return std::make_unique<const ContinueStatement>(getTerminator(), m_continueKeyword, std::move(label));
        }

        inline const Token& getContinueKeyword() const { return m_continueKeyword; }
        inline const IdentifierExpression* const getLabel() const { return m_label? m_label.get(): nullptr; }
    private:
        const Token m_continueKeyword;
        const std::unique_ptr<const IdentifierExpression> m_label;
    };
}