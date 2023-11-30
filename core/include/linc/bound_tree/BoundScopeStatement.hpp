#pragma once
#include <memory>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundScopeStatement final : public BoundStatement
    {
    public:
        BoundScopeStatement(std::vector<std::unique_ptr<const BoundStatement>>&& statements)
            :m_statements(std::move(statements))
        {}

        const std::vector<std::unique_ptr<const BoundStatement>>& getStatements() const { return m_statements; }
    private:
        const std::vector<std::unique_ptr<const BoundStatement>> m_statements;
    };
}