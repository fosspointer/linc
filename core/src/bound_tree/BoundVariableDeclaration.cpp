#include <linc/bound_tree/BoundVariableDeclaration.hpp>

namespace linc
{
    BoundVariableDeclaration::BoundVariableDeclaration(Types::type type, const std::string& name, std::unique_ptr<const BoundExpression> default_value)
        :m_actualType(type), m_name(name), m_defaultValue(std::move(default_value))
    {}

    std::unique_ptr<const BoundDeclaration> BoundVariableDeclaration::clone() const
    {
        return std::make_unique<const BoundVariableDeclaration>(m_actualType, m_name, m_defaultValue? m_defaultValue->clone(): nullptr);
    }
    
    std::string BoundVariableDeclaration::toStringInner() const
    {
        return Logger::format("Variable Declaration (=$) (::$)", PrimitiveValue(m_name), PrimitiveValue(m_actualType));
    }
}