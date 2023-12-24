#pragma once
#include <memory>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundWhileExpression final : public BoundExpression
    {
    public:
        BoundWhileExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_statement, Types::Type type)
            :BoundExpression(type), m_testExpression(std::move(test_expression)), m_bodyStatement(std::move(body_statement))
        {
            if(m_testExpression->getType() != Types::Type::_bool)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = Logger::format("While expression check expected type '$', got '$' instead.",
                        Types::toString(Types::Type::_bool), Types::toString(m_testExpression->getType()))
                });
        }

        const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundStatement* const getBodyStatement() const { return m_bodyStatement.get(); }
    private:
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundStatement> m_bodyStatement;
    };
}