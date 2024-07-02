#include <linc/bound_tree/BoundIfExpression.hpp>

#define LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK \
    if(m_testExpression->getType().primitive != Types::Kind::_bool) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT, \
            .message = Logger::format("If expression check expected type '$', got '$' instead.", \
                Types::kindToString(Types::Kind::_bool), Types::toString(m_testExpression->getType())) \
        })

namespace linc
{
    BoundIfExpression::BoundIfExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, 
        std::unique_ptr<const BoundStatement> body_else_statement, Types::type type)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement)),
        m_bodyElseStatement(std::move(body_else_statement))
    {
        LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
    }

    BoundIfExpression::BoundIfExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement,
        Types::type type)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement))
    {
        LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
    }

    std::unique_ptr<const BoundExpression> BoundIfExpression::clone() const
    {
        if(m_bodyElseStatement.has_value())
            return std::make_unique<const BoundIfExpression>(std::move(m_testExpression->clone()), std::move(m_bodyIfStatement->clone()),
                std::move(m_bodyElseStatement.value()->clone()), getType());
        
        return std::make_unique<const BoundIfExpression>(std::move(m_testExpression->clone()),
            std::move(m_bodyIfStatement->clone()), getType());
    }

    std::string BoundIfExpression::toStringInner() const
    {
        return Logger::format("If$ Expression", m_bodyElseStatement.has_value()? "/Else": "");
    }
}