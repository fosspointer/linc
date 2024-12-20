#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundNodeListClause.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundMatchClause final : public BoundClause<BoundMatchClause>
    {
    public:
        BoundMatchClause(std::unique_ptr<const BoundExpression> expression, std::unique_ptr<const BoundNodeListClause<BoundExpression>> values)
            :BoundClause(expression->getInfo()), m_expression(std::move(expression)), m_values(std::move(values))
        {}

        [[nodiscard]] inline const BoundExpression* const getExpression() const { return m_expression.get(); }
        [[nodiscard]] inline const BoundNodeListClause<BoundExpression>* const getValues() const { return m_values.get(); }

        std::unique_ptr<const BoundMatchClause> clone() const final override
        {
            return std::make_unique<const BoundMatchClause>(m_expression->clone(), m_values->clone());
        }
    private:
        const std::unique_ptr<const BoundExpression> m_expression;
        const std::unique_ptr<const BoundNodeListClause<BoundExpression>> m_values;
    };
}