#include <linc/bound_tree/BoundIfExpression.hpp>

#define LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK \
    if(m_testExpression->getType().primitive != Types::Kind::_bool) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT, \
            .message = Logger::format("If expression check expected type '$', got '$' instead.", \
                Types::kindToString(Types::Kind::_bool), m_testExpression->getType().toString()) \
        })

namespace linc
{
    BoundIfExpression::BoundIfExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression,
        std::unique_ptr<const BoundExpression> if_body, std::unique_ptr<const BoundExpression> else_body)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_ifBody(std::move(if_body)),
        m_elseBody(std::move(else_body))
    {
        LINC_BOUND_IF_ELSE_EXPRESSION_TYPECHECK;
    }

    std::unique_ptr<const BoundExpression> BoundIfExpression::clone() const
    {
        return std::make_unique<const BoundIfExpression>(getType(), m_testExpression->clone(), m_ifBody->clone(), m_elseBody? m_elseBody->clone(): nullptr);
    }

    std::string BoundIfExpression::toStringInner() const
    {
        return Logger::format("If$ Expression", m_elseBody? "+Else": std::string{});
    }
}