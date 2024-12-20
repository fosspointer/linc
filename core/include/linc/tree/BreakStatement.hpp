#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BreakStatement final : public Statement
    {
    public:
        BreakStatement(const Token& terminator, const Token& break_keyword, std::unique_ptr<const IdentifierExpression> label)
            :Statement(terminator, break_keyword.info), m_breakKeyword(break_keyword), m_label(std::move(label))
        {
            addToken(m_breakKeyword);
            
            if(m_label)
                addTokens(m_label->getTokens());
        }

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            auto label = m_label? Types::uniqueCast<const IdentifierExpression>(m_label->clone()): nullptr;
            return std::make_unique<const BreakStatement>(getTerminator(), m_breakKeyword, std::move(label));
        }

        inline const Token& getBreakKeyword() const { return m_breakKeyword; }
        inline const IdentifierExpression* const getLabel() const { return m_label? m_label.get(): nullptr; }
    private:
        const Token m_breakKeyword;
        const std::unique_ptr<const IdentifierExpression> m_label;
    };
}