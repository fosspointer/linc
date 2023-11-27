#pragma once
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundScopeStatement final : public BoundStatement
    {
    public:
        BoundScopeStatement(std::vector<const BoundStatement*> statements)
            :m_statements(std::move(statements))
        {}

        const std::vector<const BoundStatement*>& getStatements() const { return m_statements; }
    private:
        const std::vector<const BoundStatement*> m_statements;
    };
}