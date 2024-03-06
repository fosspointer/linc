#include <linc/bound_tree/BoundFunctionCallExpression.hpp>

namespace linc
{
    BoundFunctionCallExpression::BoundFunctionCallExpression(Types::type type, const std::string& name, 
        std::vector<Argument> arguments, std::unique_ptr<const BoundStatement> body)
        :BoundExpression(type), m_name(name), m_arguments(std::move(arguments)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundExpression> BoundFunctionCallExpression::cloneConst() const
    {
        std::vector<Argument> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(Argument{
                .name = argument.name,
                .value = std::move(argument.value->cloneConst()),
                .isMutable = argument.isMutable
            });

        return std::make_unique<const BoundFunctionCallExpression>(getType(), m_name, std::move(arguments), std::move(m_body->cloneConst()));
    }

    std::string BoundFunctionCallExpression::toStringInner() const
    {
        return Logger::format("Bound Function Call (='$')", m_name);
    }
}