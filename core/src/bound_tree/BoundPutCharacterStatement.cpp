#include <linc/bound_tree/BoundPutCharacterStatement.hpp>

namespace linc
{
    BoundPutCharacterStatement::BoundPutCharacterStatement(std::unique_ptr<const BoundExpression> expression)
        :BoundStatement(Types::voidType), m_expression(std::move(expression))
    {
        if(m_expression->getType().kind != Types::Kind::_char)
        {
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                .message = linc::Logger::format("Invalid type for put character statement. Expected 'char', got '$'.",
                    Types::toString(m_expression->getType()))
            });
        }
    }

    std::unique_ptr<const BoundStatement> BoundPutCharacterStatement::cloneConst() const
    {
        return std::make_unique<const BoundPutCharacterStatement>(std::move(m_expression->cloneConst()));
    }

    std::string BoundPutCharacterStatement::toStringInner() const
    {
        return "Bound Put-character Statement";
    }
}