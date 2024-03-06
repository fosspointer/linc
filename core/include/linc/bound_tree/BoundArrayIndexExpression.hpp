#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundIdentifierExpression.hpp>

namespace linc
{
    class BoundArrayIndexExpression final : public BoundExpression
    {
    public:
        BoundArrayIndexExpression(std::unique_ptr<const BoundIdentifierExpression> identifier, std::unique_ptr<const BoundExpression> index,
            const Types::type& type);

        [[nodiscard]] inline const BoundIdentifierExpression* const getIdentifier() const { return m_identifier.get(); } 
        [[nodiscard]] inline const BoundExpression* const getIndex() const { return m_index.get(); } 

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
        virtual std::string toStringInner() const final override;
    private:
        const std::unique_ptr<const BoundIdentifierExpression> m_identifier;
        const std::unique_ptr<const BoundExpression> m_index;
    };
}