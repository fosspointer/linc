#include <linc/bound_tree/BoundExpressionStatement.hpp>

namespace linc
{
    BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression)
        :BoundStatement(expression->getType()), m_expression(std::move(expression))
    {}

    std::unique_ptr<const BoundStatement> BoundExpressionStatement::clone_const() const
    {
        return std::make_unique<const BoundExpressionStatement>(std::move(m_expression->clone_const()));
    }

    std::string BoundExpressionStatement::toStringInner() const
    {
        return "Bound Expression Statement";
    }
}