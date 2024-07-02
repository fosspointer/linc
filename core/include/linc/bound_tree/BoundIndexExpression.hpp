#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    class BoundIndexExpression final : public BoundExpression
    {
    public:
        BoundIndexExpression(std::unique_ptr<const BoundExpression> identifier, std::unique_ptr<const BoundExpression> index,
            const Types::type& type);

        [[nodiscard]] inline const BoundExpression* const getArray() const { return m_array.get(); } 
        [[nodiscard]] inline const BoundExpression* const getIndex() const { return m_index.get(); } 

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
        
        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_array.get(), m_index.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::unique_ptr<const BoundExpression> m_array, m_index;
    };
}