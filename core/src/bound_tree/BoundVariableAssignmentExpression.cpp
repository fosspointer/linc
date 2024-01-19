#include <linc/bound_tree/BoundVariableAssignmentExpression.hpp>

namespace linc
{
    BoundVariableAssignmentExpression::BoundVariableAssignmentExpression(const std::string& identifier, std::unique_ptr<const BoundExpression> value)
        :BoundExpression(value->getType()), m_identifier(identifier), m_value(std::move(value))
    {}

    std::unique_ptr<const BoundExpression> BoundVariableAssignmentExpression::clone_const() const
    {
        return std::make_unique<const BoundVariableAssignmentExpression>(m_identifier, std::move(m_value->clone_const()));
    }

    std::string BoundVariableAssignmentExpression::toStringInner() const
    {
        return "Bound Variable Assignment Expression";
    }
}