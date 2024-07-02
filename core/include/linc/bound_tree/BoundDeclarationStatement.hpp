#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundDeclarationStatement final : public BoundStatement
    {
    public:
        BoundDeclarationStatement(std::unique_ptr<const BoundDeclaration> declaration);
        [[nodiscard]] inline const BoundDeclaration* const getDeclaration() const { return m_declaration.get(); }

        virtual std::unique_ptr<const BoundStatement> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_declaration.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundDeclaration> m_declaration;
    };
}