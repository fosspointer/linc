#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundVariableAssignmentExpression : public BoundExpression
    {
    public:
        BoundVariableAssignmentExpression(const std::string& identifier, std::unique_ptr<const BoundExpression> value)
            :BoundExpression(value->getType()), m_identifier(identifier), m_value(std::move(value))
        {}

        const std::string& getIdentifier() const { return m_identifier; }
        const BoundExpression* const getValue() const { return m_value.get(); }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Variable Assignment Expression";
        }

        const std::string m_identifier;
        const std::unique_ptr<const BoundExpression> m_value;
    };
}