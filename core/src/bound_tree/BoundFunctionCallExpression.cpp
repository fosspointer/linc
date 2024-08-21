#include <linc/bound_tree/BoundFunctionCallExpression.hpp>

namespace linc
{
    BoundFunctionCallExpression::BoundFunctionCallExpression(Types::type type, const std::string& name, 
        std::vector<Argument> arguments)
        :BoundExpression(type), m_name(name), m_arguments(std::move(arguments))
    {}

    std::unique_ptr<const BoundExpression> BoundFunctionCallExpression::clone() const
    {
        std::vector<Argument> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(Argument{
                .name = argument.name,
                .value = std::move(argument.value->clone())
            });

        return std::make_unique<const BoundFunctionCallExpression>(getType(), m_name, std::move(arguments));
    }

    std::string BoundFunctionCallExpression::toStringInner() const
    {
        return Logger::format("Function Call (=$)", PrimitiveValue(m_name));
    }
}