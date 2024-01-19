#include <linc/bound_tree/BoundLiteralExpression.hpp>

namespace linc
{
    TypedValue BoundLiteralExpression::getValue() const { return m_value; }

    BoundLiteralExpression::BoundLiteralExpression(const TypedValue& value)
        :BoundExpression(value.getType()), m_value(value)
    {}

    std::string BoundLiteralExpression::toStringInner() const
    {
        return Logger::format("Bound Literal Expression (='$')", m_value.toString());
    }

    std::unique_ptr<const BoundExpression> BoundLiteralExpression::clone_const() const
    {
        return std::make_unique<const BoundLiteralExpression>(m_value);
    }
}