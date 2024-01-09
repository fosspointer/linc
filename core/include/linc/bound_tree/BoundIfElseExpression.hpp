#pragma once
#include <memory>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

#define LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK \
    if(m_testExpression->getType() != Types::Type::_bool) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT, \
            .message = Logger::format("If-else expression check expected type '$', got '$' instead.", \
                Types::toString(Types::Type::_bool), Types::toString(m_testExpression->getType())) \
        })

namespace linc
{
    class BoundIfElseExpression final : public BoundExpression
    {
    public:
        BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, 
            std::unique_ptr<const BoundStatement> body_else_statement, Types::Type type)
            :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement)),
            m_bodyElseStatement(std::move(body_else_statement))
        {
            LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
        }

        BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, Types::Type type)
            :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement))
        {
            LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
        }

        const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundStatement* const getIfBodyStatement() const { return m_bodyIfStatement.get(); }
        const std::optional<const BoundStatement* const> getElseBodyStatement() const
        { 
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get();
            else return std::nullopt;
        }
        const bool hasElse() const { return m_bodyElseStatement.has_value(); }
    private:
        virtual std::string toStringInner() const final override
        {
            return Logger::format("Bound If$ Expression", m_bodyElseStatement.has_value()? "/Else": "");
        }
        
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundStatement> m_bodyIfStatement;
        const std::optional<const std::unique_ptr<const BoundStatement>> m_bodyElseStatement;
    };
}