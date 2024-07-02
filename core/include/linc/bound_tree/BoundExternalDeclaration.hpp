#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    class BoundExternalDeclaration final : public BoundDeclaration
    {
    public:
        BoundExternalDeclaration(const std::string& name, std::unique_ptr<const BoundTypeExpression> actual_type,
            std::vector<std::unique_ptr<const BoundTypeExpression>> arguments)
            :BoundDeclaration(Types::voidType), m_name(name), m_actualType(std::move(actual_type)), m_arguments(std::move(arguments))
        {}

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override
        {
            std::vector<std::unique_ptr<const BoundTypeExpression>> arguments;
            
            for(const auto& argument: m_arguments)
            {
                auto type = Types::unique_cast<const BoundTypeExpression>(argument->clone());
                arguments.push_back(std::move(type));
            }

            auto actual_type = Types::unique_cast<const BoundTypeExpression>(m_actualType->clone());

            return std::make_unique<const BoundExternalDeclaration>(m_name, std::move(actual_type), std::move(arguments));
        }

        const std::string& getName() const { return m_name; }
        const BoundTypeExpression* const getActualType() const { return m_actualType.get(); }
        const std::vector<std::unique_ptr<const BoundTypeExpression>>& getArguments() const { return m_arguments; }
    private:
        virtual std::string toStringInner() const final override
        {
            return "External Declaration";
        }

        const std::string m_name;
        const std::unique_ptr<const BoundTypeExpression> m_actualType;
        const std::vector<std::unique_ptr<const BoundTypeExpression>> m_arguments;
    };
}