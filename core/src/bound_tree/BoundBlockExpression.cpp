#include <linc/bound_tree/BoundBlockExpression.hpp>

namespace linc
{
    BoundBlockExpression::BoundBlockExpression(std::vector<std::unique_ptr<const BoundStatement>> statements)
        :BoundExpression(statements.empty()? Types::fromKind(Types::Kind::_void): statements.back()->getType()), m_statements(std::move(statements))
    {}

    std::unique_ptr<const BoundExpression> BoundBlockExpression::clone() const
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;
        statements.reserve(m_statements.size());

        for(const auto& statement: m_statements)
            statements.push_back(std::move(statement->clone()));

        return std::make_unique<const BoundBlockExpression>(std::move(statements));
    }

    std::string BoundBlockExpression::toStringInner() const
    {
        return "Block Expression";
    }
}