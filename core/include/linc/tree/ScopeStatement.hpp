#pragma once
#include <linc/tree/Statement.hpp>

namespace linc
{
    class ScopeStatement final : public Statement
    {
    public:
        ScopeStatement(Token left_brace_token, Token right_brace_token, std::vector<std::unique_ptr<const Statement>> statements)
            :Statement(NodeInfo{.tokenList = {left_brace_token}, .lineNumber = left_brace_token.lineNumber}), 
            m_leftBraceToken(left_brace_token), m_rightBraceToken(right_brace_token), m_statements(std::move(statements))
        {
            for(const auto& statement: m_statements)
                addTokens(statement->getTokens());
            addToken(m_rightBraceToken);
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            std::vector<const Node*> nodes;
            for(const auto& statement: m_statements)
                nodes.push_back(statement.get());
            
            return nodes;
        }

        inline const std::vector<std::unique_ptr<const Statement>>& getStatements() const { return m_statements; }
        inline const Statement* getStatement(std::vector<const Statement*>::size_type index) const { return m_statements.at(index).get(); }
        
        virtual std::unique_ptr<const Statement> clone_const() const final override
        {
            std::vector<std::unique_ptr<const Statement>> statements_clone;
            for(const auto& statement: m_statements)
                statements_clone.push_back(std::move(statement->clone_const()));

            return std::make_unique<const ScopeStatement>(m_leftBraceToken, m_rightBraceToken, std::move(statements_clone));
        }
    private:
        const Token m_leftBraceToken, m_rightBraceToken;
        const std::vector<std::unique_ptr<const Statement>> m_statements;
    };
}