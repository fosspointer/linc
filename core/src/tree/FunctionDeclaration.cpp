#include <linc/tree/FunctionDeclaration.hpp>
#include <linc/tree/FunctionPrototypeDeclaration.hpp>
#include <linc/tree/VariableDeclaration.hpp>

namespace linc
{
    FunctionDeclaration::FunctionDeclaration(std::unique_ptr<const FunctionPrototypeDeclaration> prototype, std::unique_ptr<const Expression> body)
        :Declaration(Types::uniqueCast<const IdentifierExpression>(prototype->getIdentifier()->clone())), m_prototype(std::move(prototype)), m_body(std::move(body))
    {
        addTokens(m_prototype->getTokens());
        addTokens(m_body->getTokens());
    }
    
    FunctionDeclaration::~FunctionDeclaration() = default;

    std::unique_ptr<const Declaration> FunctionDeclaration::clone() const
    {
        auto prototype = Types::uniqueCast<const FunctionPrototypeDeclaration>(m_prototype->clone());
        return std::make_unique<const FunctionDeclaration>(std::move(prototype), m_body->clone());
    }

    std::unique_ptr<const Declaration> FunctionDeclaration::cloneRename(std::unique_ptr<const IdentifierExpression> identifier) const
    {
        auto prototype = Types::uniqueCast<const FunctionPrototypeDeclaration>(m_prototype->cloneRename(Types::uniqueCast<const IdentifierExpression>(identifier->clone())));
        return std::make_unique<const FunctionDeclaration>(std::move(prototype), m_body->clone());
    }
}