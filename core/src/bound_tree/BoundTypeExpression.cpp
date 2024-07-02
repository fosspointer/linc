#include <linc/bound_tree/BoundTypeExpression.hpp>

namespace linc
{
    BoundTypeExpression::BoundTypeExpression(Types::type::Primitive primitive, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_base(primitive), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}
    
    BoundTypeExpression::BoundTypeExpression(Types::type::Structure structure, bool is_mutable, BoundArraySpecifiers specifiers)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_base(std::move(structure)), m_isMutable(is_mutable), m_arraySpecifiers(std::move(specifiers))
    {}
    
    std::unique_ptr<const BoundExpression> BoundTypeExpression::clone() const
    {
        if(auto primitive = std::get_if<Types::type::Primitive>(&m_base))
            return std::make_unique<const BoundTypeExpression>(*primitive, m_isMutable, m_arraySpecifiers);

        else
        {
            const auto& structure = std::get<Types::type::Structure>(m_base);
            Types::type::Structure new_structure;
            new_structure.reserve(structure.size());

            for(const auto& element: structure)
                new_structure.push_back(std::pair(element.first, element.second->clone()));
            
            return std::make_unique<const BoundTypeExpression>(std::move(new_structure), m_isMutable, m_arraySpecifiers);
        }
    }
    
    std::string BoundTypeExpression::toStringInner() const
    {
        return Logger::format("Type Expression (=$)", Types::toString(getActualType()));
    }
}