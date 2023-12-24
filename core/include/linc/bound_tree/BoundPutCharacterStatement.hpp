#pragma once
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class BoundPutCharacterStatement final : public BoundStatement
    {
    public:
        BoundPutCharacterStatement(std::unique_ptr<const BoundExpression> expression)
            :BoundStatement(Types::Type::_void), m_expression(std::move(expression))
        {
            if(m_expression->getType() != Types::Type::_char)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = linc::Logger::format("Invalid type for put character statement. Expected 'char', got '$'.",
                        Types::toString(m_expression->getType()))
                });
            }
        }

        const BoundExpression* const getExpression() const { return m_expression.get(); }
    private:
        const std::unique_ptr<const BoundExpression> m_expression;
    };
}