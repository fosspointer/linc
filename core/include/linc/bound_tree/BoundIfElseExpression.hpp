#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundIfElseExpression final : public BoundExpression
    {
    public:
        BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, 
            std::unique_ptr<const BoundStatement> body_else_statement, Types::type type);

        BoundIfElseExpression(std::unique_ptr<const BoundExpression> test_expression, std::unique_ptr<const BoundStatement> body_if_statement, Types::type type);

        [[nodiscard]] inline const BoundExpression* const getTestExpression() const { return m_testExpression.get(); }
        [[nodiscard]] inline const BoundStatement* const getIfBodyStatement() const { return m_bodyIfStatement.get(); }
        [[nodiscard]] inline const std::optional<const BoundStatement* const> getElseBodyStatement() const
        { 
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get();
            else return std::nullopt;
        }
        [[nodiscard]] inline const bool hasElse() const { return m_bodyElseStatement.has_value(); }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;
        
        const std::unique_ptr<const BoundExpression> m_testExpression;
        const std::unique_ptr<const BoundStatement> m_bodyIfStatement;
        const std::optional<const std::unique_ptr<const BoundStatement>> m_bodyElseStatement;
    };
}