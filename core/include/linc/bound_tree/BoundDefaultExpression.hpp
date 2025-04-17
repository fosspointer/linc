#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundDefaultExpression final : public BoundExpression
    {
    public:
        BoundDefaultExpression(const Types::type& type)
            :BoundExpression(type)
        {}

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
    };
}