#pragma once
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BreakStatement final : public Statement
    {
    public:
        BreakStatement(const Token& break_keyword, std::unique_ptr<const IdentifierExpression> label)
            :Statement(break_keyword.info), m_breakKeyword(break_keyword), m_label(std::move(label))
        {}

        virtual std::unique_ptr<const Statement> clone() const final override
        {
            auto label = m_label? Types::unique_cast<const IdentifierExpression>(m_label->clone()): nullptr;
            return std::make_unique<const BreakStatement>(m_breakKeyword, std::move(label));
        }

        inline const Token& getBreakKeyword() const { return m_breakKeyword; }
        inline const IdentifierExpression* const getLabel() const { return m_label.get(); }
    private:
        const Token m_breakKeyword;
        const std::unique_ptr<const IdentifierExpression> m_label;
    };
}