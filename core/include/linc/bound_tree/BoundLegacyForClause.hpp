#pragma once
#include <linc/Include.hpp>
#include <linc/bound_tree/BoundClause.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundDeclaration.hpp>

namespace linc
{
    class BoundLegacyForClause final : public BoundClause<BoundLegacyForClause>
    {
    public:
        BoundLegacyForClause(std::unique_ptr<const BoundDeclaration> declaration, std::unique_ptr<const BoundExpression> test_expression,
            std::unique_ptr<const BoundExpression> end_expression)
            :BoundClause<BoundLegacyForClause>(declaration->getInfo()), m_declaration(std::move(declaration)), m_testExpression(std::move(test_expression)),
            m_endExpression(std::move(end_expression))
        {}

        inline const BoundDeclaration* const getDeclaration() const { return m_declaration.get(); }
        inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        inline const BoundExpression* const getEndExpression() const { return m_endExpression.get(); }

        std::unique_ptr<const BoundLegacyForClause> clone() const final override
        {
            return std::make_unique<const BoundLegacyForClause>(m_declaration->clone(), m_testExpression->clone(), m_endExpression->clone());
        }
    private:
        const std::unique_ptr<const BoundDeclaration> m_declaration;
        const std::unique_ptr<const BoundExpression> m_testExpression, m_endExpression;
    };
}