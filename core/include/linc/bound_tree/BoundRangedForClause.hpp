#pragma once
#include <linc/Include.hpp>
#include <linc/bound_tree/BoundClause.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    class BoundRangedForClause final : public BoundClause<BoundRangedForClause>
    {
    public:
        BoundRangedForClause(std::unique_ptr<const BoundIdentifierExpression> identifier, std::unique_ptr<const BoundExpression> expression)
            :BoundClause<BoundRangedForClause>(identifier->getInfo()), m_identifier(std::move(identifier)), m_expression(std::move(expression))
        {}

        inline const BoundIdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const BoundExpression* const getExpression() const { return m_expression.get(); }

        std::unique_ptr<const BoundRangedForClause> clone() const final override
        {
            auto identifier = Types::uniqueCast<const BoundIdentifierExpression>(m_identifier->clone());
            return std::make_unique<const BoundRangedForClause>(std::move(identifier), m_expression->clone());
        }
    private:
        const std::unique_ptr<const BoundIdentifierExpression> m_identifier;
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}