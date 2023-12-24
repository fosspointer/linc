#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundPutStringStatement final : public BoundStatement
    {
    public:
        BoundPutStringStatement(std::unique_ptr<const BoundExpression> expression)
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

        const BoundExpression* const getExpression() const { return m_expression.get(); }
    private:
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}