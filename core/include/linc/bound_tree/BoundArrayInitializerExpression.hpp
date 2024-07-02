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
        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    
        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            std::vector<const BoundNode*> children;

            for(const auto& value: m_values)
                children.push_back(value.get());
            
            return children;
        }
    private:
        std::vector<std::unique_ptr<const BoundExpression>> m_values;
    };
}