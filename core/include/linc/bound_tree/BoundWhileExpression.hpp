#pragma once
#include <memory>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundWhileExpression final : public BoundExpression
    {
    public:
        BoundWhileExpression(Types::Type type, std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_while_statement, 
        std::optional<std::unique_ptr<const BoundStatement>> body_finally_statement = {}, std::optional<std::unique_ptr<const BoundStatement>> body_else_statement = {})
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

        const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundStatement* const getWhileBodyStatement() const { return m_bodyWhileStatement.get(); }

        const std::optional<const BoundStatement* const> getFinallyBodyStatement() const 
        { 
            if(m_bodyFinallyStatement.has_value())
                return m_bodyFinallyStatement.value().get(); 
            else return std::nullopt;
        }
        
        const std::optional<const BoundStatement* const> getElseBodyStatement() const
        {
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get(); 
            else return std::nullopt;
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return Logger::format("Bound While$:$ Expression", m_bodyFinallyStatement.has_value()? "/Finally": "",
                m_bodyElseStatement.has_value()? "/Else": "");
        }
        
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundStatement> m_bodyWhileStatement;
        const std::optional<const std::unique_ptr<const BoundStatement>> m_bodyFinallyStatement, m_bodyElseStatement;
    };
}