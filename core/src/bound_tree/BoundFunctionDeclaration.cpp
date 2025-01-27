#include <linc/bound_tree/BoundFunctionDeclaration.hpp>

namespace linc
{
    BoundFunctionDeclaration::BoundFunctionDeclaration(const Types::type& function_type, const std::string& name, 
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments, 
        std::unique_ptr<const BoundExpression> body)
        :BoundDeclaration(name), m_functionType(function_type), m_arguments(std::move(arguments)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundDeclaration> BoundFunctionDeclaration::clone() const
    {
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(std::make_unique<const BoundVariableDeclaration>(
                argument->getActualType(), argument->getName(),
                    argument->getDefaultValue()? argument->getDefaultValue()->clone(): nullptr
            ));

        return std::make_unique<const BoundFunctionDeclaration>(getFunctionType(), m_name, std::move(arguments), std::move(m_body->clone()));
    }

    std::string BoundFunctionDeclaration::toStringInner() const
    {
        return Logger::format("Function Declaration (=$) (:$)", PrimitiveValue(m_name), PrimitiveValue(m_functionType));
    }
}