#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundVariableAssignmentExpression final : public BoundExpression
    {
    public:
        BoundVariableAssignmentExpression(const std::string& identifier, std::unique_ptr<const BoundExpression> value);

        [[nodiscard]] inline const std::string& getIdentifier() const { return m_identifier; }
        [[nodiscard]] inline const BoundExpression* const getValue() const { return m_value.get(); }

        virtual std::unique_ptr<const BoundExpression> clone_const() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_identifier;
        const std::unique_ptr<const BoundExpression> m_value;
    };
}