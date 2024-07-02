#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    BoundIdentifierExpression::BoundIdentifierExpression(const std::string& value, const Types::type type)
        :BoundExpression(type), m_value(value)
    {}

    std::unique_ptr<const BoundExpression> BoundIdentifierExpression::clone() const
    {
        return std::make_unique<const BoundIdentifierExpression>(m_value, getType());
    }

    const std::string& BoundIdentifierExpression::getValue() const { return m_value; }
    
    std::string BoundIdentifierExpression::toStringInner() const
    {
        return Logger::format("Identifier Expression (=$)", PrimitiveValue(m_value));
    }
    
    const std::string m_value;
}