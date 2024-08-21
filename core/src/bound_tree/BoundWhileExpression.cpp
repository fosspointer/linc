#include <linc/bound_tree/BoundWhileExpression.hpp>

namespace linc
{
    BoundWhileExpression::BoundWhileExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundExpression> while_body, 
        std::optional<std::unique_ptr<const BoundExpression>> finally_body, std::optional<std::unique_ptr<const BoundExpression>> else_body)
        :BoundExpression(type), m_testExpression(std::move(test_expression)), m_whileBody(std::move(while_body)),
        m_finallyBody(finally_body? std::make_optional(std::move(finally_body.value())): std::nullopt),
        m_elseBody(else_body? std::make_optional(std::move(else_body.value())): std::nullopt)
    {
        if(!m_testExpression->getType().isCompatible(Types::fromKind(Types::Kind::_bool)))
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = Logger::format("While expression check expected type '$', got '$' instead.",
                    Types::toString(Types::fromKind(Types::Kind::_bool)), Types::toString(m_testExpression->getType()))
            });
    }

    std::unique_ptr<const BoundExpression> BoundWhileExpression::clone() const
    {
        if(m_finallyBody.has_value())
            if(m_elseBody.has_value())
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone()), 
                    std::move(m_whileBody->clone()), std::move(m_finallyBody.value()->clone()),
                    std::move(m_elseBody.value()->clone()));
            else
                return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone()),
                    std::move(m_whileBody->clone()), std::move(m_finallyBody.value()->clone()),
                    std::nullopt);
        else
            return std::make_unique<const BoundWhileExpression>(getType(), std::move(m_testExpression->clone()),
                std::move(m_whileBody->clone()), std::nullopt, std::nullopt);
    }

    std::string BoundWhileExpression::toStringInner() const
    {
        return Logger::format("While$:$ Expression", m_finallyBody.has_value()? "/Finally": "",
            m_elseBody.has_value()? "/Else": "");
    }
}