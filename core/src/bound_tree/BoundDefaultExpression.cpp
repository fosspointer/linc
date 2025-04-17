#include <linc/bound_tree/BoundDefaultExpression.hpp>

namespace linc
{
    std::unique_ptr<const BoundExpression> BoundDefaultExpression::clone() const
    {
        return std::make_unique<const BoundDefaultExpression>(getType());
    }


    std::string BoundDefaultExpression::toStringInner() const
    {
        return Logger::format("Default Expression (::$)", getType());
    }
}