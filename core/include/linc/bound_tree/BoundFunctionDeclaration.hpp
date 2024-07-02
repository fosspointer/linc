#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundVariableDeclaration.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundFunctionDeclaration final : public BoundDeclaration
    {
    public:
        BoundFunctionDeclaration(const Types::type& type, const std::string& name, std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments, 
            std::unique_ptr<const BoundStatement> body);

        [[nodiscard]] inline Types::type getReturnType() const { return m_returnType; }
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundVariableDeclaration>>& getArguments() const { return m_arguments; }
        [[nodiscard]] inline const BoundStatement* const getBody() const { return m_body.get(); }

        [[nodiscard]] inline auto getDefaultArgumentCount() const
        {
            std::vector<std::unique_ptr<const BoundVariableDeclaration>>::size_type count{};
            
            for(const auto& argument: m_arguments)
                if(argument->getDefaultValue().has_value())
                    ++count;
            
            return count;
        }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override 
        {
            std::vector<const BoundNode*> nodes;
            for(const auto& argument: m_arguments)
                nodes.push_back(argument.get());
            
            nodes.push_back(m_body.get());
            return std::move(nodes);
        }
    private:
        virtual std::string toStringInner() const final override;

        const Types::type m_returnType;
        const std::string m_name;
        const std::vector<std::unique_ptr<const BoundVariableDeclaration>> m_arguments;
        const std::unique_ptr<const BoundStatement> m_body;
    };
}