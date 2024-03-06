#include <linc/bound_tree/BoundFunctionDeclaration.hpp>

namespace linc
{
    BoundFunctionDeclaration::BoundFunctionDeclaration(const Types::type& type, const std::string& name, 
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments, 
        std::unique_ptr<const BoundStatement> body)
        :BoundDeclaration(Types::voidType), m_returnType(type), m_name(name), m_arguments(std::move(arguments)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundDeclaration> BoundFunctionDeclaration::cloneConst() const
    {
        std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(std::make_unique<const BoundVariableDeclaration>(
                argument->getActualType(), argument->getName(),
                    argument->getDefaultValue().has_value()? 
                    std::make_optional(std::move(argument->getDefaultValue().value()->cloneConst())):
                    std::nullopt
            ));

        return std::make_unique<const BoundFunctionDeclaration>(getReturnType(), m_name, std::move(arguments), std::move(m_body->cloneConst()));
    }

    std::string BoundFunctionDeclaration::toStringInner() const
    {
        return "Bound Function Declaration";
    }
}