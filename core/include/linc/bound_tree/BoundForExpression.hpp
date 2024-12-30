#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundVariantClause.hpp>
#include <linc/bound_tree/BoundLegacyForClause.hpp>
#include <linc/bound_tree/BoundRangedForClause.hpp>

namespace linc
{
    class BoundForExpression final : public BoundExpression 
    {
    public:
        using ForClause = BoundVariantClause<BoundLegacyForClause, BoundRangedForClause>;
        BoundForExpression(std::string_view label, std::unique_ptr<const ForClause> clause, std::unique_ptr<const BoundExpression> body);

        [[nodiscard]] inline const std::string& getLabel() const { return m_label; }
        [[nodiscard]] inline const ForClause* const getForClause() const { return m_clause.get(); }
        [[nodiscard]] inline const BoundExpression* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
        
        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            return {m_body.get()};
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::string m_label;
        const std::unique_ptr<const ForClause> m_clause;
        const std::unique_ptr<const BoundExpression> m_body;
    };
}