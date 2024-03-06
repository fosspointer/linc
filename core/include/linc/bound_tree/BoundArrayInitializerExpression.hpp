#pragma once
#include <linc/Include.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundArrayInitializerExpression final : public BoundExpression
    {
    public:
        BoundArrayInitializerExpression(std::vector<std::unique_ptr<const BoundExpression>> values, Types::type type);

        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundExpression>>& getValues() const { return m_values; }

        virtual std::string toStringInner() const final override;
        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        std::vector<std::unique_ptr<const BoundExpression>> m_values;
    };
}