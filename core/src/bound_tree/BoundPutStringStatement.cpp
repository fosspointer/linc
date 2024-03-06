#include <linc/bound_tree/BoundPutStringStatement.hpp>

namespace linc
{
    BoundPutStringStatement::BoundPutStringStatement(std::unique_ptr<const BoundExpression> expression)
        :BoundStatement(Types::fromKind(Types::Kind::_void)), m_expression(std::move(expression))
    {
        if(m_expression->getType().kind != Types::Kind::string)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Invalid type for put string statement. Expected 'string', got '$'.",
                    Types::toString(m_expression->getType()))
            });
        }
    }

    std::unique_ptr<const BoundStatement> BoundPutStringStatement::cloneConst() const
    {
        return std::make_unique<const BoundPutStringStatement>(std::move(m_expression->cloneConst()));
    }
    
    std::string BoundPutStringStatement::toStringInner() const
    {
        return "Bound Put-string Statement";
    }
}