#include <linc/bound_tree/BoundScopeStatement.hpp>

namespace linc
{
    BoundScopeStatement::BoundScopeStatement(std::vector<std::unique_ptr<const BoundStatement>> statements)
        :BoundStatement(statements.empty()? Types::fromKind(Types::Kind::_void): statements.back()->getType()), m_statements(std::move(statements))
    {}

    std::unique_ptr<const BoundStatement> BoundScopeStatement::cloneConst() const
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;
        for(const auto& statement: m_statements)
            statements.push_back(std::move(statement->cloneConst()));

        return std::make_unique<const BoundScopeStatement>(std::move(statements));
    }

    std::string BoundScopeStatement::toStringInner() const
    {
        return "Bound Scope Statement";
    }
}