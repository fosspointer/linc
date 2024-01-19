#include <linc/bound_tree/BoundVariableDeclaration.hpp>

namespace linc
{
    BoundVariableDeclaration::BoundVariableDeclaration(Types::Type type, const std::string& name, bool is_mutable, std::unique_ptr<const BoundExpression> value_expression)
        :BoundDeclaration(Types::Type::_void), m_type(type), m_name(name), m_valueExpression(std::move(value_expression)), m_mutable(is_mutable)
    {}

    std::unique_ptr<const BoundDeclaration> BoundVariableDeclaration::clone_const() const
    {
        return std::make_unique<const BoundVariableDeclaration>(m_type, m_name, m_mutable, std::move(m_valueExpression->clone_const()));
    }
    
    std::string BoundVariableDeclaration::toStringInner() const
    {
        return "Bound Variable Declaration";
    }
}