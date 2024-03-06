#include <linc/bound_tree/BoundWhileExpression.hpp>

namespace linc
{
    BoundWhileExpression::BoundWhileExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_while_statement, 
        std::optional<std::unique_ptr<const BoundStatement>> body_finally_statement, std::optional<std::unique_ptr<const BoundStatement>> body_else_statement)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyWhileStatement(std::move(body_while_statement)),
        m_bodyElseStatement(body_else_statement? std::make_optional(std::move(body_else_statement.value())): std::nullopt),
        m_bodyFinallyStatement(body_finally_statement? std::make_optional(std::move(body_finally_statement.value())): std::nullopt)
    {
        if(m_testExpression->getType().kind != Types::Kind::_bool)
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("While expression check expected type '$', got '$' instead.",
                    Types::toString(Types::fromKind(Types::Kind::_bool)), Types::toString(m_testExpression->getType()))
            });
    }

    std::unique_ptr<const BoundExpression> BoundWhileExpression::cloneConst() const
    {
        if(m_bodyFinallyStatement.has_value())
            if(m_bodyElseStatement.has_value())
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->cloneConst()), 
                    std::move(m_bodyWhileStatement->cloneConst()), std::move(m_bodyFinallyStatement.value()->cloneConst()),
                    std::move(m_bodyElseStatement.value()->cloneConst()));
            else
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->cloneConst()),
                    std::move(m_bodyWhileStatement->cloneConst()), std::move(m_bodyFinallyStatement.value()->cloneConst()),
                    std::nullopt);
        else
            return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->cloneConst()),
                std::move(m_bodyWhileStatement->cloneConst()), std::nullopt, std::nullopt);
    }

    std::string BoundWhileExpression::toStringInner() const
    {
        return Logger::format("Bound While$:$ Expression", m_bodyFinallyStatement.has_value()? "/Finally": "",
            m_bodyElseStatement.has_value()? "/Else": "");
    }
}