#include <linc/bound_tree/BoundForExpression.hpp>

namespace linc
{

    BoundForExpression::BoundForExpression(std::string_view label, std::unique_ptr<const ForClause> clause, std::unique_ptr<const BoundExpression> body)
        :BoundExpression(body->getType()), m_label(label), m_clause(std::move(clause)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundExpression> BoundForExpression::clone() const 
    {
        return std::make_unique<const BoundForExpression>(m_label, m_clause->clone(), m_body->clone());
    }

    std::string BoundForExpression::toStringInner() const 
    {
        return "For Expression";
    }
}