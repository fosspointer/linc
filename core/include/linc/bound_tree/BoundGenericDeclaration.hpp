#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundGenericDeclaration final : public BoundDeclaration
    {
    public:
        BoundGenericDeclaration(std::unique_ptr<const Declaration> declaration, std::vector<std::string> type_identifiers, std::size_t instance_map_index)
            :BoundDeclaration(declaration->getIdentifier()->getValue()), m_declaration(std::move(declaration)), m_typeIdentifiers(std::move(type_identifiers)),
            m_instanceMapIndex(instance_map_index)
        {}

        [[nodiscard]] inline const Declaration* const getDeclaration() const { return m_declaration.get(); }
        [[nodiscard]] inline const std::vector<std::string>& getTypeIdentifiers() const { return m_typeIdentifiers; }
        [[nodiscard]] inline std::size_t getInstanceMapIndex() const { return m_instanceMapIndex; }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override
        {
            return std::make_unique<const BoundGenericDeclaration>(m_declaration->clone(), m_typeIdentifiers, m_instanceMapIndex);
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Generic Declaration";
        }

        const std::unique_ptr<const Declaration> m_declaration;
        const std::vector<std::string> m_typeIdentifiers;
        std::size_t m_instanceMapIndex;
    };
}
