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
            std::vector<std::unique_ptr<const BoundTypeExpression>> arguments);

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;

        const BoundTypeExpression* const getActualType() const { return m_actualType.get(); }
        const std::vector<std::unique_ptr<const BoundTypeExpression>>& getArguments() const { return m_arguments; }
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundTypeExpression> m_actualType;
        const std::vector<std::unique_ptr<const BoundTypeExpression>> m_arguments;
    };
}