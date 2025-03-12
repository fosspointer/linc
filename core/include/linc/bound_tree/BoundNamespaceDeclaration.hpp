#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>

namespace linc
{
    class BoundNamespaceDeclaration final : public BoundDeclaration
    {
    public:
        BoundNamespaceDeclaration(const std::string& name, std::vector<std::unique_ptr<const BoundDeclaration>> declarations);
        ~BoundNamespaceDeclaration();

        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundDeclaration>>& getDeclarations() const { return m_declarations; }
        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;
        virtual std::vector<const BoundNode*> getChildren() const final override;
    private:
        [[nodiscard]] inline virtual std::string toStringInner() const final override { return Logger::format("Namespace Declaration (=$)", m_name); }
        const std::vector<std::unique_ptr<const BoundDeclaration>> m_declarations;
    };
}