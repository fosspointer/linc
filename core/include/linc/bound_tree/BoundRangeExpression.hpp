#pragma once
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundRangeExpression final : public BoundExpression
    {
    public:
        BoundRangeExpression(std::unique_ptr<const BoundExpression> begin_expression, std::unique_ptr<const BoundExpression> end_expression, const Types::type& type)
            :BoundExpression(type), m_begin(std::move(begin_expression)), m_end(std::move(end_expression))
        {}

        [[nodiscard]] inline const BoundExpression* const getBegin() const { return m_begin.get(); }
        [[nodiscard]] inline const BoundExpression* const getEnd() const { return m_end.get(); }

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            return std::make_unique<const BoundRangeExpression>(m_begin->clone(), m_end->clone(), getType());
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Range Expression";
        }
        const std::unique_ptr<const BoundExpression> m_begin, m_end;
    };
}