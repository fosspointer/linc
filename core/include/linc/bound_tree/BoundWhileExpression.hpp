#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundWhileExpression final : public BoundExpression
    {
    public:
        BoundWhileExpression(Types::type type, std::unique_ptr<const BoundExpression> test_expression,
            std::unique_ptr<const BoundStatement> body_while_statement, 
            std::optional<std::unique_ptr<const BoundStatement>> body_finally_statement = {},
            std::optional<std::unique_ptr<const BoundStatement>> body_else_statement = {});

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        const BoundStatement* const getWhileBodyStatement() const { return m_bodyWhileStatement.get(); }

        [[nodiscard]] inline const std::optional<const BoundStatement* const> getFinallyBodyStatement() const 
        { 
            if(m_bodyFinallyStatement.has_value())
                return m_bodyFinallyStatement.value().get(); 
            else return std::nullopt;
        }
        
        [[nodiscard]] inline const std::optional<const BoundStatement* const> getElseBodyStatement() const
        {
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get(); 
            else return std::nullopt;
        }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundStatement> m_bodyWhileStatement;
        const std::optional<const std::unique_ptr<const BoundStatement>> m_bodyFinallyStatement, m_bodyElseStatement;
    };
}