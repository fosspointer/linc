#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    BoundIdentifierExpression::BoundIdentifierExpression(const std::string& value, const Types::Type type)
        :BoundExpression(type), m_value(value)
    {}

    std::unique_ptr<const BoundExpression> BoundIdentifierExpression::clone_const() const
    {
        return std::make_unique<const BoundIdentifierExpression>(m_value, getType());
    }

    const std::string& BoundIdentifierExpression::getValue() const { return m_value; }
    
    std::string BoundIdentifierExpression::toStringInner() const
    {
        return Logger::format("Bound Identifier Expression (='$')", m_value);
    }
    
    const std::string m_value;
}