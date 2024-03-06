#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundIdentifierExpression final : public BoundExpression
    {
    public:
        BoundIdentifierExpression(const std::string& value, const Types::type type);
        [[nodiscard]] const std::string& getValue() const;

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_value;
    };
}