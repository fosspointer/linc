#include <linc/bound_tree/BoundExternalCallExpression.hpp>

namespace linc
{
    BoundExternalCallExpression::BoundExternalCallExpression(Types::type type, const std::string& name, 
        std::vector<std::unique_ptr<const BoundExpression>> arguments)
        :BoundExpression(type), m_name(name), m_arguments(std::move(arguments))
    {}

    std::unique_ptr<const BoundExpression> BoundExternalCallExpression::clone() const
    {
        std::vector<std::unique_ptr<const BoundExpression>> arguments;
        arguments.reserve(m_arguments.size());

        for(const auto& argument: m_arguments)
            arguments.push_back(argument->clone());

        return std::make_unique<const BoundExternalCallExpression>(getType(), m_name, std::move(arguments));
    }

    std::string BoundExternalCallExpression::toStringInner() const
    {
        return Logger::format("External Function Call (=$)", PrimitiveValue(m_name));
    }
}