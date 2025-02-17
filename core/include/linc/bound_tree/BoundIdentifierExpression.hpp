#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundIdentifierExpression final : public BoundExpression
    {
    public:
        BoundIdentifierExpression(const std::string& value, const Types::type type, std::size_t scope_index = 0ul);
        [[nodiscard]] const std::string& getValue() const;
        [[nodiscard]] inline std::size_t getScopeIndex() const { return m_scopeIndex; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_value;
        const std::size_t m_scopeIndex;
    };
}