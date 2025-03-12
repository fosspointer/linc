#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundDeclaration : public BoundNode
    {
    public:
        BoundDeclaration(const std::string& name)
            :m_name(name) {}
        
        BoundDeclaration(std::string_view name)
            :m_name(name) {}

        virtual ~BoundDeclaration() = default;
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        virtual std::unique_ptr<const BoundDeclaration> clone() const = 0;
        virtual std::unique_ptr<const BoundDeclaration> cloneRename(const std::string& name) const
        {
            auto declaration = clone();
            declaration->m_name = name;
            return declaration;
        }
    protected:
        mutable std::string m_name;
    };
}