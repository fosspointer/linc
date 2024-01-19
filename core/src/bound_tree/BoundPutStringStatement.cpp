#include <linc/bound_tree/BoundPutStringStatement.hpp>

namespace linc
{
    BoundPutStringStatement::BoundPutStringStatement(std::unique_ptr<const BoundExpression> expression)
        :BoundStatement(Types::Type::_void), m_expression(std::move(expression))
    {
        if(m_expression->getType() != Types::Type::string)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Invalid type for put string statement. Expected 'string', got '$'.",
                    Types::toString(m_expression->getType()))
            });
        }
    }

    std::unique_ptr<const BoundStatement> BoundPutStringStatement::clone_const() const
    {
        return std::make_unique<const BoundPutStringStatement>(std::move(m_expression->clone_const()));
    }
    
    std::string BoundPutStringStatement::toStringInner() const
    {
        return "Bound Put-string Statement";
    }
}