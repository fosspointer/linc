#include <linc/bound_tree/BoundFunctionCallExpression.hpp>

namespace linc
{
    BoundFunctionCallExpression::BoundFunctionCallExpression(Types::type type, std::unique_ptr<const BoundExpression> function,
        std::vector<std::unique_ptr<const BoundExpression>> arguments)
        :BoundExpression(type), m_function(std::move(function)), m_arguments(std::move(arguments))
    {}

    std::vector<const BoundNode*> BoundFunctionCallExpression::getChildren() const
    {
        std::vector<const BoundNode*> children;
        children.reserve(m_arguments.size() + 1ul);
        children.push_back(m_function.get());
        for(const auto& argument: m_arguments)
            children.push_back(argument.get());
        return children; 
    }

    std::unique_ptr<const BoundExpression> BoundFunctionCallExpression::clone() const
    {
        std::vector<std::unique_ptr<const BoundExpression>> arguments;
        arguments.reserve(m_arguments.size());
        for(const auto& argument: m_arguments)
            arguments.push_back(argument->clone());

        return std::make_unique<const BoundFunctionCallExpression>(getType(), m_function->clone(), std::move(arguments));
    }

    std::string BoundFunctionCallExpression::toStringInner() const
    {
        return Logger::format("Function Call (::$)", PrimitiveValue(m_function->clone()->getType()));
    }
}