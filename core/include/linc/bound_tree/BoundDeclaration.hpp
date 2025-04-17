#pragma once
#include <linc/bound_tree/BoundNode.hpp>
#include <linc/system/Memory.hpp>

namespace linc
{
    class BoundDeclaration : public BoundNode
    {
    public:
        using DeprecatedMessage = std::optional<std::string_view>;
        BoundDeclaration(const std::string& name, DeprecatedMessage deprecated_message)
            :m_name(name), m_deprecatedMessage(std::move(deprecated_message)) {}
        
        BoundDeclaration(std::string_view name, DeprecatedMessage deprecated_message)
            :m_name(name), m_deprecatedMessage(std::move(deprecated_message)) {}

        virtual ~BoundDeclaration() = default;
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const DeprecatedMessage& getDeprecatedMessage() const { return m_deprecatedMessage; }
        virtual std::unique_ptr<const BoundDeclaration> clone() const = 0;
        virtual std::unique_ptr<const BoundDeclaration> cloneRename(const std::string& name) const
        {
            auto declaration = clone();
            declaration->m_name = name;
            return declaration;
        }
    protected:
        mutable std::string m_name;
        const DeprecatedMessage m_deprecatedMessage;
    };
}