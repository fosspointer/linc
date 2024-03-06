#include <linc/bound_tree/BoundExpressionStatement.hpp>

namespace linc
{
    BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<const BoundExpression> expression)
        :BoundStatement(expression->getType()), m_expression(std::move(expression))
    {}

    std::unique_ptr<const BoundStatement> BoundExpressionStatement::cloneConst() const
    {
        return std::make_unique<const BoundExpressionStatement>(std::move(m_expression->cloneConst()));
    }

    std::string BoundExpressionStatement::toStringInner() const
    {
        return "Bound Expression Statement";
    }
}