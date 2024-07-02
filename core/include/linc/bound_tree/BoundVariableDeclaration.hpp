#pragma once
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>

namespace linc
{
    class BoundVariableDeclaration final : public BoundDeclaration
    {
    public:
        BoundVariableDeclaration(Types::type type, const std::string& name, std::optional<std::unique_ptr<const BoundExpression>> default_value);

        [[nodiscard]] inline const Types::type& getActualType() const { return m_actualType; }
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::optional<const BoundExpression* const> getDefaultValue() const
        {
            return m_defaultValue.has_value()?
                std::make_optional(m_defaultValue.value().get()): std::nullopt;
        }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override 
        {
            if(m_defaultValue.has_value())
                return {m_defaultValue.value().get()};
            else return {};
        }
    private:
        virtual std::string toStringInner() const final override;
        const Types::type m_actualType;
        const std::string m_name;
        const std::optional<std::unique_ptr<const BoundExpression>> m_defaultValue;
    };
}