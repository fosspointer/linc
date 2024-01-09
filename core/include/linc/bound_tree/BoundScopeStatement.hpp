#pragma once
#include <memory>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundScopeStatement final : public BoundStatement
    {
    public:
        BoundScopeStatement(std::vector<std::unique_ptr<const BoundStatement>> statements)
            :BoundStatement(statements.empty()? Types::Type::_void: statements.at(statements.size() - 1)->getType()), m_statements(std::move(statements))
        {}

        const std::vector<std::unique_ptr<const BoundStatement>>& getStatements() const { return m_statements; }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Scope Statement";
        }
        
        const std::vector<std::unique_ptr<const BoundStatement>> m_statements;
    };
}