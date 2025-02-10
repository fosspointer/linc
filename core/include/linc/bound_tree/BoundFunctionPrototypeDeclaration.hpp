#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundNodeListClause.hpp>

namespace linc
{
    class BoundVariableDeclaration;
    class BoundFunctionPrototypeDeclaration final : public BoundDeclaration
    {
    public:
        BoundFunctionPrototypeDeclaration(const Types::type& function_type, const std::string& name, std::unique_ptr<const BoundNodeListClause<BoundVariableDeclaration>> arguments);
        ~BoundFunctionPrototypeDeclaration();

        [[nodiscard]] inline const Types::type& getReturnType() const { return *m_functionType.function.returnType; }
        [[nodiscard]] inline const Types::type& getFunctionType() const { return m_functionType; }
        [[nodiscard]] inline const std::unique_ptr<const BoundNodeListClause<BoundVariableDeclaration>>& getArguments() const { return m_arguments; }
        [[nodiscard]] std::size_t getDefaultArgumentCount() const;

        virtual std::vector<const BoundNode*> getChildren() const final override;
        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const Types::type m_functionType;
        const std::unique_ptr<const BoundNodeListClause<BoundVariableDeclaration>> m_arguments;
    };
}