#include <linc/bound_tree/BoundArrayIndexExpression.hpp>

namespace linc
{
    BoundArrayIndexExpression::BoundArrayIndexExpression(std::unique_ptr<const BoundIdentifierExpression> identifier, std::unique_ptr<const BoundExpression> index,
        const Types::type& type)
        :BoundExpression(type), m_identifier(std::move(identifier)), m_index(std::move(index))
    {}

    std::unique_ptr<const BoundExpression> BoundArrayIndexExpression::cloneConst() const
    {
        auto identifier = Types::unique_cast<const BoundIdentifierExpression>(m_identifier->cloneConst());

        return std::make_unique<const BoundArrayIndexExpression>(std::move(identifier), m_index->cloneConst(), getType());
    }

    std::string BoundArrayIndexExpression::toStringInner() const
    {
        return "Bound Array Index Expression";
    }
}