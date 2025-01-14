#include <linc/bound_tree/BoundTypeExpression.hpp>

namespace linc
{
    BoundTypeExpression::BoundTypeExpression(Types::type::Primitive primitive, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_root(primitive), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}
    
    BoundTypeExpression::BoundTypeExpression(Types::type::Structure structure, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_root(std::move(structure)), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}

    BoundTypeExpression::BoundTypeExpression(Types::type::Enumeration enumeration, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_root(std::move(enumeration)), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}

    BoundTypeExpression::BoundTypeExpression(Types::type::Function function, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_root(std::move(function)), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}
    
    std::unique_ptr<const BoundExpression> BoundTypeExpression::clone() const
    {
        if(auto primitive = std::get_if<Types::type::Primitive>(&m_root))
            return std::make_unique<const BoundTypeExpression>(*primitive, m_isMutable, std::move(m_arraySpecifiers));
        
        else if(auto enumeration = std::get_if<Types::type::Enumeration>(&m_root))
            return std::make_unique<const BoundTypeExpression>(*enumeration, m_isMutable, std::move(m_arraySpecifiers));

        else if(auto function = std::get_if<Types::type::Function>(&m_root))
        {
            Types::type::Function function_clone;
            function_clone.returnType = function->returnType->clone();
            function_clone.argumentTypes = function->argumentTypes;

            return std::make_unique<const BoundTypeExpression>(std::move(function_clone), m_isMutable, std::move(m_arraySpecifiers));
        }
        else
        {
            const auto& structure = std::get<Types::type::Structure>(m_root);
            return std::make_unique<const BoundTypeExpression>(structure, m_isMutable, std::move(m_arraySpecifiers));
        }
    }
    
    std::string BoundTypeExpression::toStringInner() const
    {
        return Logger::format("Type Expression (=$)", getActualType().toString());
    }
}