#include <linc/bound_tree/BoundNamespaceDeclaration.hpp>

namespace linc
{
    BoundNamespaceDeclaration::BoundNamespaceDeclaration(const std::string& name, std::vector<std::unique_ptr<const BoundDeclaration>> declarations)
        :BoundDeclaration(name), m_declarations(std::move(declarations))
    {}
    BoundNamespaceDeclaration::~BoundNamespaceDeclaration() = default;
    
    std::vector<const BoundNode*> BoundNamespaceDeclaration::getChildren() const
    {
        std::vector<const BoundNode*> children;
        children.reserve(m_declarations.size());
        for(const auto& declaration: m_declarations)
            children.push_back(declaration.get());
        return children;    
    }

    std::unique_ptr<const BoundDeclaration> BoundNamespaceDeclaration::clone() const
    {
        std::vector<std::unique_ptr<const BoundDeclaration>> declarations;
        declarations.reserve(m_declarations.size());
        for(const auto& declaration: m_declarations)
            declarations.push_back(declaration->clone());
        return std::make_unique<const BoundNamespaceDeclaration>(m_name, std::move(declarations));
    }
}