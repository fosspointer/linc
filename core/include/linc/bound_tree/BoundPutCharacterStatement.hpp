#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundPutCharacterStatement final : public BoundStatement
    {
    public:
        BoundPutCharacterStatement(std::unique_ptr<const BoundExpression> expression);
        [[nodiscard]] const BoundExpression* const getExpression() const { return m_expression.get(); }

        virtual std::unique_ptr<const BoundStatement> clone_const() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}