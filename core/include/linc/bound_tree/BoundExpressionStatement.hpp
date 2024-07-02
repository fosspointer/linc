#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundExpressionStatement final : public BoundStatement
    {
    public:
        BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression);
        [[nodiscard]] inline const BoundExpression* const getExpression() const { return m_expression.get(); } 

        virtual std::unique_ptr<const BoundStatement> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_expression.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}