#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundAccessExpression final : public BoundExpression
    {
    public:
        BoundAccessExpression(std::unique_ptr<const BoundExpression> base, Types::u64 index, const Types::type& type)
            :BoundExpression(type), m_base(std::move(base)), m_index(index)
        {}

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            return std::make_unique<const BoundAccessExpression>(m_base->clone(), m_index, getType());
        }

        [[nodiscard]] inline const BoundExpression* const getBase() const { return m_base.get(); }
        [[nodiscard]] inline Types::u64 getIndex() const { return m_index; }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Access Expression";
        }
        const std::unique_ptr<const BoundExpression> m_base; 
        const Types::u64 m_index;
    };
}