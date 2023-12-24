#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    class WhileExpression final : public Expression
    {
    public:
        WhileExpression(const Token& while_keyword_token, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Statement> body_statement)
            :m_whileKeywordToken(while_keyword_token), m_testExpression(std::move(test_expression)), m_bodyStatement(std::move(body_statement))
        {}

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_testExpression.get(), m_bodyStatement.get()};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const WhileExpression>(m_whileKeywordToken,
                std::move(m_testExpression->clone_const()), std::move(m_bodyStatement->clone_const()));
        }

        const Token& getWhileKeywordToken() const { return m_whileKeywordToken; }
        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        const Statement* const getBodyStatement() const { return m_bodyStatement.get(); }  
    private:
        const Token m_whileKeywordToken;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const Statement> m_bodyStatement;
    };
}