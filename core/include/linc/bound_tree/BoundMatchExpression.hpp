#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundMatchClause.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundMatchExpression final : public BoundExpression
    {
    public:
        BoundMatchExpression(std::unique_ptr<const BoundExpression> test_expression,
            std::unique_ptr<const BoundNodeListClause<BoundMatchClause>> clauses, const Types::type& type)
            :BoundExpression(type), m_testExpression(std::move(test_expression)), m_clauses(std::move(clauses))
        {}

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            return std::make_unique<const BoundMatchExpression>(m_testExpression->clone(), m_clauses->clone(), getType());
        }

        virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_testExpression.get()};
        }

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        [[nodiscard]] inline const BoundNodeListClause<BoundMatchClause>* const getClauses() const { return m_clauses.get(); }
    private:
        virtual std::string toStringInner() const final override { return "Match Expression"; }
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundNodeListClause<BoundMatchClause>> m_clauses;
    };
}