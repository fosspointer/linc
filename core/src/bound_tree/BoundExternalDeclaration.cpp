#include <linc/bound_tree/BoundExternalDeclaration.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    BoundExternalDeclaration::BoundExternalDeclaration(const std::string& name, std::unique_ptr<const BoundTypeExpression> actual_type,
        std::vector<std::unique_ptr<const BoundTypeExpression>> arguments)
        :m_name(name), m_actualType(std::move(actual_type)), m_arguments(std::move(arguments))
    {}

    std::unique_ptr<const BoundDeclaration> BoundExternalDeclaration::clone() const
    {
        std::vector<std::unique_ptr<const BoundTypeExpression>> arguments;
        
        for(const auto& argument: m_arguments)
        {
            auto type = Types::uniqueCast<const BoundTypeExpression>(argument->clone());
            arguments.push_back(std::move(type));
        }

        auto actual_type = Types::uniqueCast<const BoundTypeExpression>(m_actualType->clone());

        return std::make_unique<const BoundExternalDeclaration>(m_name, std::move(actual_type), std::move(arguments));
    }
    std::string BoundExternalDeclaration::toStringInner() const
    {
        return "External Declaration";
    }
}