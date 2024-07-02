#include <linc/bound_tree/BoundIndexExpression.hpp>

namespace linc
{
    BoundIndexExpression::BoundIndexExpression(std::unique_ptr<const BoundExpression> array, std::unique_ptr<const BoundExpression> index,
        const Types::type& type)
        :BoundExpression(type), m_array(std::move(array)), m_index(std::move(index))
    {}

    std::unique_ptr<const BoundExpression> BoundIndexExpression::clone() const
    {
        return std::make_unique<const BoundIndexExpression>(m_array->clone(), m_index->clone(), getType());
    }

    std::string BoundIndexExpression::toStringInner() const
    {
        return "Index Expression";
    }
}