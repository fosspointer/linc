#pragma once
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundScopeStatement final : public BoundStatement
    {
    public:
        BoundScopeStatement(std::vector<std::unique_ptr<const BoundStatement>> statements);
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundStatement>>& getStatements() const { return m_statements; }

        virtual std::unique_ptr<const BoundStatement> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const std::vector<std::unique_ptr<const BoundStatement>> m_statements;
    };
}