#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundPutStringStatement final : public BoundStatement
    {
    public:
        BoundPutStringStatement(std::unique_ptr<const BoundExpression> expression);
        [[nodiscard]] inline const BoundExpression* const getExpression() const { return m_expression.get(); }

        virtual std::unique_ptr<const BoundStatement> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}