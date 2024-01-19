#include <linc/bound_tree/BoundWhileExpression.hpp>

namespace linc
{
    BoundWhileExpression::BoundWhileExpression(Types::Type type, std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_while_statement, 
        std::optional<std::unique_ptr<const BoundStatement>> body_finally_statement, std::optional<std::unique_ptr<const BoundStatement>> body_else_statement)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyWhileStatement(std::move(body_while_statement)),
        m_bodyElseStatement(body_else_statement.has_value()? std::move(body_else_statement.value())
            : std::optional<const std::unique_ptr<const BoundStatement>>()),
        m_bodyFinallyStatement(body_finally_statement.has_value()? std::move(body_finally_statement.value())
            : std::optional<const std::unique_ptr<const BoundStatement>>())
    {
        if(m_testExpression->getType() != Types::Type::_bool)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("While expression check expected type '$', got '$' instead.",
                    Types::toString(Types::Type::_bool), Types::toString(m_testExpression->getType()))
            });
    }

    std::unique_ptr<const BoundExpression> BoundWhileExpression::clone_const() const
    {
        if(m_bodyFinallyStatement.has_value())
            if(m_bodyElseStatement.has_value())
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone_const()), 
                    std::move(m_bodyWhileStatement->clone_const()), std::move(m_bodyFinallyStatement.value()->clone_const()),
                    std::move(m_bodyElseStatement.value()->clone_const()));
            else
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone_const()),
                    std::move(m_bodyWhileStatement->clone_const()), std::move(m_bodyFinallyStatement.value()->clone_const()),
                    std::nullopt);
        else
            return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone_const()),
                std::move(m_bodyWhileStatement->clone_const()), std::nullopt, std::nullopt);
    }

    std::string BoundWhileExpression::toStringInner() const
    {
        return Logger::format("Bound While$:$ Expression", m_bodyFinallyStatement.has_value()? "/Finally": "",
            m_bodyElseStatement.has_value()? "/Else": "");
    }
}