#include <linc/bound_tree/BoundFunctionDeclaration.hpp>

namespace linc
{
    BoundFunctionDeclaration::BoundFunctionDeclaration(const Types::type& function_type, const std::string& name, 
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments, 
        std::unique_ptr<const BoundExpression> body)
        :m_functionType(function_type), m_name(name), m_arguments(std::move(arguments)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundDeclaration> BoundFunctionDeclaration::clone() const
    {
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(std::make_unique<const BoundVariableDeclaration>(
                argument->getActualType(), argument->getName(),
                    argument->getDefaultValue().has_value()? 
                    std::make_optional(std::move(argument->getDefaultValue().value()->clone())):
                    std::nullopt
            ));

        return std::make_unique<const BoundFunctionDeclaration>(getFunctionType(), m_name, std::move(arguments), std::move(m_body->clone()));
    }

    std::string BoundFunctionDeclaration::toStringInner() const
    {
        return Logger::format("Function Declaration (=$) (:$)", PrimitiveValue(m_name), PrimitiveValue(m_functionType));
    }
}