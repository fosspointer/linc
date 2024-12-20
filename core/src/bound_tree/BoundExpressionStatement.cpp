#include <linc/bound_tree/BoundExpressionStatement.hpp>

namespace linc
{
    BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression)
        :m_expression(std::move(expression))
    {}

    std::unique_ptr<const BoundStatement> BoundExpressionStatement::clone() const
    {
        return std::make_unique<const BoundExpressionStatement>(std::move(m_expression->clone()));
    }

    std::string BoundExpressionStatement::toStringInner() const
    {
        return "Expression Statement";
    }
}