#pragma once
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundDeclaration.hpp>

namespace linc
{
    class BoundVariableDeclaration final : public BoundDeclaration
    {
    public:
        BoundVariableDeclaration(Types::Type type, const std::string& name, bool is_mutable, std::unique_ptr<const BoundExpression> value_expression);

        [[nodiscard]] inline Types::Type getType() const { return m_type; }
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const BoundExpression* const getValueExpression() const { return m_valueExpression.get(); }
        [[nodiscard]] inline bool getMutable() const { return m_mutable; }

        virtual std::unique_ptr<const BoundDeclaration> clone_const() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const Types::Type m_type;
        const std::string m_name;
        const bool m_mutable;
        const std::unique_ptr<const BoundExpression> m_valueExpression;
    };
}