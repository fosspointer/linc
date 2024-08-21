#include <linc/bound_tree/BoundBlockExpression.hpp>

namespace linc
{
    BoundBlockExpression::BoundBlockExpression(std::vector<std::unique_ptr<const BoundStatement>> statements, std::unique_ptr<const BoundExpression> tail)
        :BoundExpression(tail? tail->getType(): Types::fromKind(Types::Kind::_void)),
        m_statements(std::move(statements)),
        m_tail(std::move(tail))
    {}

    std::unique_ptr<const BoundExpression> BoundBlockExpression::clone() const
    {
        std::vector<std::unique_ptr<const BoundStatement>> statements;
        statements.reserve(m_statements.size());

        for(const auto& statement: m_statements)
            statements.push_back(std::move(statement->clone()));

        auto tail = m_tail? m_tail->clone(): nullptr;
        return std::make_unique<const BoundBlockExpression>(std::move(statements), std::move(tail));
    }

    std::string BoundBlockExpression::toStringInner() const
    {
        return "Block Expression";
    }
}