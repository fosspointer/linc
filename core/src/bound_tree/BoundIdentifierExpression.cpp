#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    BoundIdentifierExpression::BoundIdentifierExpression(const std::string& value, const Types::type type, std::size_t scope_index)
        :BoundExpression(type), m_value(value), m_scopeIndex(scope_index)
    {}

    std::unique_ptr<const BoundExpression> BoundIdentifierExpression::clone() const
    {
        return std::make_unique<const BoundIdentifierExpression>(m_value, getType(), m_scopeIndex);
    }

    const std::string& BoundIdentifierExpression::getValue() const { return m_value; }
    
    std::string BoundIdentifierExpression::toStringInner() const
    {
        return Logger::format("Identifier Expression (=$)", PrimitiveValue(m_value));
    }
    
    const std::string m_value;
}