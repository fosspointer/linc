#include <linc/bound_tree/BoundArrayInitializerExpression.hpp>

namespace linc
{
    BoundArrayInitializerExpression::BoundArrayInitializerExpression(std::vector<std::unique_ptr<const BoundExpression>> values, Types::type type)
        :BoundExpression(type), m_values(std::move(values))
    {}

    std::string BoundArrayInitializerExpression::toStringInner() const
    {
        return "Bound Array Initializer Expression";
    }

    std::unique_ptr<const BoundExpression> BoundArrayInitializerExpression::cloneConst() const
    {
        std::vector<std::unique_ptr<const BoundExpression>> values;

        for(const auto& value: m_values)
            values.push_back(value->cloneConst());

        return std::make_unique<const BoundArrayInitializerExpression>(std::move(values), getType());
    }
}