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
    protected:
        const std::string m_name;
    };
}