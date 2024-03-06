#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    class WhileExpression final : public Expression
    {
    public:
        WhileExpression(const Token& while_keyword_token, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Statement> body_while_statement,
            const std::optional<Token>& finally_keyword_token = {}, std::optional<std::unique_ptr<const Statement>> body_finally_statement = {},
            const std::optional<Token>& else_keyword_token = {}, std::optional<std::unique_ptr<const Statement>> body_else_statement = {})
            :m_whileKeywordToken(while_keyword_token), m_testExpression(std::move(test_expression)), m_bodyWhileStatement(std::move(body_while_statement)),
            m_finallyKeywordToken(finally_keyword_token), m_elseKeywordToken(else_keyword_token),
            m_bodyFinallyStatement(body_finally_statement.has_value()? std::move(body_finally_statement.value())
                : std::optional<const std::unique_ptr<const Statement>>()),
            m_bodyElseStatement(body_else_statement.has_value()? std::move(body_else_statement.value())
                : std::optional<const std::unique_ptr<const Statement>>())
        {

            addToken(m_whileKeywordToken);
            addTokens(m_testExpression->getTokens());
            addTokens(m_bodyWhileStatement->getTokens());

            if(m_bodyFinallyStatement.has_value())
                addTokens(m_bodyFinallyStatement->get()->getTokens());
            
            if(m_bodyElseStatement.has_value())
                addTokens(m_bodyElseStatement->get()->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            if(m_finallyKeywordToken.has_value())
                if(m_elseKeywordToken.has_value())
                    return {m_testExpression.get(), m_bodyWhileStatement.get(), m_bodyFinallyStatement.value().get(), m_bodyElseStatement.value().get()};
                else
                    return {m_testExpression.get(), m_bodyWhileStatement.get(), m_bodyFinallyStatement.value().get()};
            else if(m_elseKeywordToken.has_value())
                return {m_testExpression.get(), m_bodyWhileStatement.get(), m_bodyElseStatement.value().get()};
            else
                return {m_testExpression.get(), m_bodyWhileStatement.get()};
            
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            return std::make_unique<const WhileExpression>(m_whileKeywordToken,
                std::move(m_testExpression->cloneConst()), std::move(m_bodyWhileStatement->cloneConst()), m_finallyKeywordToken, 
                m_bodyFinallyStatement.has_value()? std::move(m_bodyFinallyStatement.value()->cloneConst()): std::optional<std::unique_ptr<const Statement>>(),
                m_elseKeywordToken,
                m_bodyElseStatement.has_value()? std::move(m_bodyElseStatement.value()->cloneConst()): std::optional<std::unique_ptr<const Statement>>());
        }

        const Token& getWhileKeywordToken() const { return m_whileKeywordToken; }
        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        
        const Statement* const getWhileBodyStatement() const { return m_bodyWhileStatement.get(); }

        const std::optional<const Statement* const> getFinallyBodyStatement() const
        {
            if(m_bodyFinallyStatement.has_value())
                return m_bodyFinallyStatement.value().get();
            else return std::nullopt;
        }  
        
        const std::optional<const Statement* const> getElseBodyStatement() const
        {
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get();
            else return std::nullopt;
        } 
    private:
        const Token m_whileKeywordToken;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const Statement> m_bodyWhileStatement;
        std::optional<const Token> m_finallyKeywordToken, m_elseKeywordToken;
        std::optional<const std::unique_ptr<const Statement>> m_bodyFinallyStatement, m_bodyElseStatement;
    };
}