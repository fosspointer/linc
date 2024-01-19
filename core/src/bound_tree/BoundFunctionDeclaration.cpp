#include <linc/bound_tree/BoundFunctionDeclaration.hpp>

namespace linc
{
    BoundFunctionDeclaration::BoundFunctionDeclaration(Types::Type type, const std::string& name, 
        std::vector<std::unique_ptr<const BoundArgumentDeclaration>> arguments, 
        std::unique_ptr<const BoundStatement> body)
        :BoundDeclaration(Types::Type::_void), m_returnType(type), m_name(name), m_arguments(std::move(arguments)), m_body(std::move(body))
    {}

    std::unique_ptr<const BoundDeclaration> BoundFunctionDeclaration::clone_const() const
    {
        std::vector<std::unique_ptr<const BoundArgumentDeclaration>> arguments;
        for(const auto& argument: m_arguments)
            arguments.push_back(std::make_unique<const BoundArgumentDeclaration>(
                argument->getType(), argument->getName(), argument->getMutable(), 
                    argument->getDefaultValue().has_value()? std::move(argument->getDefaultValue().value()->clone_const()):
                    std::optional<std::unique_ptr<const BoundExpression>>{}
            ));

        return std::make_unique<const BoundFunctionDeclaration>(getReturnType(), m_name, std::move(arguments), std::move(m_body->clone_const()));
    }

    std::string BoundFunctionDeclaration::toStringInner() const
    {
        return "Bound Function Declaration";
    }
}