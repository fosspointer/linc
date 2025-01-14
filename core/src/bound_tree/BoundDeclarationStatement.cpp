#include <linc/bound_tree/BoundDeclarationStatement.hpp>

namespace linc
{
    BoundDeclarationStatement::BoundDeclarationStatement(std::unique_ptr<const BoundDeclaration> declaration)
        :m_declaration(std::move(declaration))
    {}

    std::unique_ptr<const BoundStatement> BoundDeclarationStatement::clone() const
    {
        return std::make_unique<const BoundDeclarationStatement>(m_declaration->clone());
    }

    std::string BoundDeclarationStatement::toStringInner() const
    {
        return "Declaration Statement";
    }
}