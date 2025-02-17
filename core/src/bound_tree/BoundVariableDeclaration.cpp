#include <linc/bound_tree/BoundVariableDeclaration.hpp>

namespace linc
{
    BoundVariableDeclaration::BoundVariableDeclaration(Types::type type, const std::string& name, std::unique_ptr<const BoundExpression> default_value, std::size_t scope_index)
        :BoundDeclaration(name), m_actualType(type), m_defaultValue(std::move(default_value)), m_scopeIndex(scope_index)
    {}

    std::unique_ptr<const BoundDeclaration> BoundVariableDeclaration::clone() const
    {
        return std::make_unique<const BoundVariableDeclaration>(m_actualType, m_name, m_defaultValue? m_defaultValue->clone(): nullptr, m_scopeIndex);
    }
    
    std::string BoundVariableDeclaration::toStringInner() const
    {
        return Logger::format("Variable Declaration (=$) (::$)", PrimitiveValue(m_name), PrimitiveValue(m_actualType));
    }
}