#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    class IfElseExpression final : public Expression
    {
    public:
        IfElseExpression(const Token& if_keyword_token, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Statement> body_if_statement,
            const Token& else_keyword_token, std::unique_ptr<const Statement> body_else_statement)
            :m_ifKeywordToken(if_keyword_token), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement)),
            m_elseKeywordToken(else_keyword_token), m_bodyElseStatement(std::move(body_else_statement))
        {
            addToken(m_ifKeywordToken);
            addTokens(m_testExpression->getTokens());
            addTokens(m_bodyIfStatement->getTokens());
            addToken(m_elseKeywordToken.value());
            addTokens(m_bodyElseStatement.value()->getTokens());
        }

        IfElseExpression(const Token& if_keyword_token, std::unique_ptr<const Expression> test_expression, std::unique_ptr<const Statement> body_if_statement)
            :m_ifKeywordToken(if_keyword_token), m_testExpression(std::move(test_expression)), m_bodyIfStatement(std::move(body_if_statement)), 
            m_elseKeywordToken(std::nullopt), m_bodyElseStatement(std::nullopt)
        {
            addToken(m_ifKeywordToken);
            addTokens(m_testExpression->getTokens());
            addTokens(m_bodyIfStatement->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            if(m_bodyElseStatement.has_value())
                return {m_testExpression.get(), m_bodyIfStatement.get(), m_bodyElseStatement->get() };
            else
                return {m_testExpression.get(), m_bodyIfStatement.get()};
        }

        virtual std::unique_ptr<const Expression> cloneConst() const final override
        {
            if(m_bodyElseStatement.has_value() && m_elseKeywordToken.has_value())
                return std::make_unique<const IfElseExpression>(m_ifKeywordToken, std::move(m_testExpression->cloneConst()), 
                    std::move(m_bodyIfStatement->cloneConst()), m_elseKeywordToken.value(), std::move(m_bodyElseStatement.value()->cloneConst()));
            else return std::make_unique<const IfElseExpression>(m_ifKeywordToken, std::move(m_testExpression->cloneConst()), 
                    std::move(m_bodyIfStatement->cloneConst()));
        }

        const Token& getIfKeywordToken() const { return m_ifKeywordToken; } 
        const std::optional<const Token>& getElseKeywordToken() const { return m_elseKeywordToken; }

        const Expression* const getTestExpression() const { return m_testExpression.get(); } 
        const Statement* const getIfBodyStatement() const { return m_bodyIfStatement.get(); } 
        const std::optional<const Statement*> getElseBodyStatement() const 
        {  
            if(m_bodyElseStatement.has_value())
                return m_bodyElseStatement.value().get();
            else return std::nullopt;
        } 
    private:
        const Token m_ifKeywordToken;
        const std::unique_ptr<const Expression> m_testExpression;
        const std::unique_ptr<const Statement> m_bodyIfStatement;
        const std::optional<const Token> m_elseKeywordToken;
        const std::optional<const std::unique_ptr<const Statement>> m_bodyElseStatement;
    };
}