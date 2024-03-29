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

        virtual std::unique_ptr<const BoundStatement> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundDeclaration> m_declaration;
    };
}