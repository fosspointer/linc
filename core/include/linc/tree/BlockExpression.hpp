#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BlockExpression final : public Expression
    {
    public:
        BlockExpression(Token left_brace_token, Token right_brace_token, std::vector<std::unique_ptr<const Statement>> statements)
            :Expression(NodeInfo{.tokenList = {left_brace_token}, .info = left_brace_token.info}), 
            m_leftBraceToken(left_brace_token), m_rightBraceToken(right_brace_token), m_statements(std::move(statements))
        {
            for(const auto& statement: m_statements)
                addTokens(statement->getTokens());
            addToken(m_rightBraceToken);
        }

        inline const std::vector<std::unique_ptr<const Statement>>& getStatements() const { return m_statements; }
        inline const Statement* getStatement(std::vector<const Statement*>::size_type index) const { return m_statements.at(index).get(); }
        
        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<std::unique_ptr<const Statement>> statements;
            statements.reserve(m_statements.size());

            for(const auto& statement: m_statements)
                statements.push_back(std::move(statement->clone()));

            return std::make_unique<const BlockExpression>(m_leftBraceToken, m_rightBraceToken, std::move(statements));
        }
    private:
        const Token m_leftBraceToken, m_rightBraceToken;
        const std::vector<std::unique_ptr<const Statement>> m_statements;
    };
}