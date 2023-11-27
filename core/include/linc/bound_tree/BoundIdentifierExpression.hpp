#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundIdentifierExpression final : public BoundExpression
    {
    public:
        BoundIdentifierExpression(const std::string& value, const Types::Type type)
            :BoundExpression(type), m_value(value)
        {}

        const std::string& getValue() const { return m_value; }
    private:
        const std::string m_value;
    };
}