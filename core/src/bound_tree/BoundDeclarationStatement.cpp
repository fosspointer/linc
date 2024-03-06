#include <linc/bound_tree/BoundDeclarationStatement.hpp>

namespace linc
{
    BoundDeclarationStatement::BoundDeclarationStatement(std::unique_ptr<const BoundDeclaration> declaration)
        :BoundStatement(declaration->getType()), m_declaration(std::move(declaration))
    {}

    std::unique_ptr<const BoundStatement> BoundDeclarationStatement::cloneConst() const
    {
        return std::make_unique<const BoundDeclarationStatement>(std::move(m_declaration->cloneConst()));
    }

    std::string BoundDeclarationStatement::toStringInner() const
    {
        return "Bound Declaration Statement";
    }
}