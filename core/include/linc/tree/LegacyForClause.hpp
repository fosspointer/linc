#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Clause.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/Declaration.hpp>

namespace linc
{
    class LegacyForClause final : public Clause<LegacyForClause>
    {
    public:
        LegacyForClause(const Token& first_terminator, const Token& second_terminator, std::unique_ptr<const Declaration> declaration,
            std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Expression> end_expression)
            :Clause<LegacyForClause>(declaration->getTokenInfo()), m_firstTerminator(first_terminator), m_secondTerminator(second_terminator),
            m_declaration(std::move(declaration)), m_testExpression(std::move(test_expression)), m_endExpression(std::move(end_expression))
        {
            addTokens(m_declaration->getTokens());
            addToken(m_firstTerminator);
            addTokens(m_testExpression->getTokens());
            addToken(m_secondTerminator);
            addTokens(m_endExpression->getTokens());
        }

        inline const Token& getFirstTerminator() const { return m_firstTerminator; }
        inline const Token& getSecondTerminator() const { return m_secondTerminator; }
        inline const Declaration* const getDeclaration() const { return m_declaration.get(); }
        inline const Expression* const getTestExpression() const { return m_testExpression.get(); }
        inline const Expression* const getEndExpression() const { return m_endExpression.get(); }

        std::unique_ptr<const LegacyForClause> clone() const final override
        {
            return std::make_unique<const LegacyForClause>(m_firstTerminator, m_secondTerminator,
                m_declaration->clone(), m_testExpression->clone(), m_endExpression->clone());
        }
    private:
        const Token m_firstTerminator, m_secondTerminator;
        const std::unique_ptr<const Declaration> m_declaration;
        const std::unique_ptr<const Expression> m_testExpression, m_endExpression;
    };
}