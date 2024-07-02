#include <linc/bound_tree/BoundLiteralExpression.hpp>

namespace linc
{
    PrimitiveValue BoundLiteralExpression::getValue() const { return m_value; }

    BoundLiteralExpression::BoundLiteralExpression(const PrimitiveValue& value, const Types::type& type)
        :BoundExpression(type), m_value(value)
    {}

    std::string BoundLiteralExpression::toStringInner() const
    {
        return Logger::format("Literal Expression (='$')", m_value.toString());
    }

    std::unique_ptr<const BoundExpression> BoundLiteralExpression::clone() const
    {
        return std::make_unique<const BoundLiteralExpression>(m_value, getType());
    }
}