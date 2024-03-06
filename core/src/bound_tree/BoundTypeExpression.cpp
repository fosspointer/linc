#include <linc/bound_tree/BoundTypeExpression.hpp>

namespace linc
{
    BoundTypeExpression::BoundTypeExpression(Types::Kind kind, bool is_mutable, bool is_array, const std::optional<Types::u64>& array_size)
        :BoundExpression(Types::fromKind(Types::Kind::type)), m_kind(kind), m_isMutable(is_mutable), m_isArray(is_array), m_arraySize(array_size)
    {}
    
    std::unique_ptr<const BoundExpression> BoundTypeExpression::cloneConst() const
    {
        return std::make_unique<BoundTypeExpression>(m_kind, m_isMutable, m_isArray, m_arraySize);
    }
    
    std::string BoundTypeExpression::toStringInner() const
    {
        return Logger::format("Bound Type Expression (=$)", Types::toString(getActualType()));
    }
}