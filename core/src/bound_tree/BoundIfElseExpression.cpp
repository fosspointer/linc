#include <linc/bound_tree/BoundIfElseExpression.hpp>

#define LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK \
    if(m_testExpression->getType().kind != Types::Kind::_bool) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT, \
            .message = Logger::format("If-else expression check expected type '$', got '$' instead.", \
                Types::kindToString(Types::Kind::_bool), Types::toString(m_testExpression->getType())) \
        })

namespace linc
{
    BoundIfElseExpression::BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, 
        std::unique_ptr<const BoundStatement> body_else_statement, Types::type type)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement)),
        m_bodyElseStatement(std::move(body_else_statement))
    {
        LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
    }

    BoundIfElseExpression::BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement,
        Types::type type)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement))
    {
        LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
    }

    std::unique_ptr<const BoundExpression> BoundIfElseExpression::cloneConst() const
    {
        if(m_bodyElseStatement.has_value())
            return std::make_unique<const BoundIfElseExpression>(std::move(m_testExpression->cloneConst()), std::move(m_bodyIfStatement->cloneConst()),
                std::move(m_bodyElseStatement.value()->cloneConst()), getType());
        
        return std::make_unique<const BoundIfElseExpression>(std::move(m_testExpression->cloneConst()),
            std::move(m_bodyIfStatement->cloneConst()), getType());
    }

    std::string BoundIfElseExpression::toStringInner() const
    {
        return Logger::format("Bound If$ Expression", m_bodyElseStatement.has_value()? "/Else": "");
    }
}