#include <linc/bound_tree/BoundWhileExpression.hpp>

namespace linc
{
    BoundWhileExpression::BoundWhileExpression(std::string_view label, Types::type type, std::unique_ptr<const BoundExpression> test_expression,
        std::unique_ptr<const BoundExpression> while_body, std::unique_ptr<const BoundExpression> finally_body,
        std::unique_ptr<const BoundExpression> else_body)
        :BoundExpression(type), m_label(label), m_testExpression(std::move(test_expression)), m_whileBody(std::move(while_body)), m_finallyBody(std::move(finally_body)),
        m_elseBody(std::move(else_body))
    {
        if(!m_testExpression->getType().isCompatible(Types::fromKind(Types::Kind::_bool)))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("While expression check expected type '$', got '$' instead.",
                    Types::fromKind(Types::Kind::_bool).toString(), m_testExpression->getType().toString())
            });
    }

    std::unique_ptr<const BoundExpression> BoundWhileExpression::clone() const
    {
        auto test_expression = m_testExpression->clone();
        auto while_body = m_whileBody->clone();
        auto finally_body = m_finallyBody? m_finallyBody->clone(): nullptr;
        auto else_body = m_elseBody? m_elseBody->clone(): nullptr;

        return std::make_unique<const BoundWhileExpression>(m_label, getType(), std::move(test_expression), std::move(while_body), std::move(finally_body),
            std::move(else_body));
    }

    std::string BoundWhileExpression::toStringInner() const
    {
        return Logger::format("While$:$ Expression", m_finallyBody? "+Finally": std::string{},
            m_elseBody? "+Else": std::string{});
    }
}