#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundBlockExpression final : public BoundExpression
    {
    public:
        BoundBlockExpression(std::vector<std::unique_ptr<const BoundStatement>> statements, std::unique_ptr<const BoundExpression> tail);
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundStatement>>& getStatements() const { return m_statements; }
        [[nodiscard]] inline const BoundExpression* const getTail() const { return m_tail? m_tail.get(): nullptr; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;

        inline virtual std::vector<const BoundNode*> getChildren() const final override
        {
            std::vector<const BoundNode*> nodes;
            for(const auto& statement: m_statements)
                nodes.push_back(statement.get());
            
            return nodes;
        }
    private:
        virtual std::string toStringInner() const final override;
        const std::vector<std::unique_ptr<const BoundStatement>> m_statements;
        const std::unique_ptr<const BoundExpression> m_tail;
    };
}