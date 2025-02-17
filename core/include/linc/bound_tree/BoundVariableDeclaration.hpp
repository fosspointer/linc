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
        BoundVariableDeclaration(Types::type type, const std::string& name, std::unique_ptr<const BoundExpression> default_value, std::size_t scope_index);

        [[nodiscard]] inline const Types::type& getActualType() const { return m_actualType; }
        [[nodiscard]] inline const BoundExpression* const getDefaultValue() const { return m_defaultValue? m_defaultValue.get(): nullptr; }
        [[nodiscard]] inline std::size_t getScopeIndex() const { return m_scopeIndex; }

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override 
        {
            return m_defaultValue? std::vector<const BoundNode*>{m_defaultValue.get()}: std::vector<const BoundNode*>{};
        }
    private:
        virtual std::string toStringInner() const final override;
        const Types::type m_actualType;
        const std::unique_ptr<const BoundExpression> m_defaultValue;
        std::size_t m_scopeIndex;
    };
}