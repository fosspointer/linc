#pragma once
#include <linc/tree/Statement.hpp>

namespace linc
{
    class ScopeStatement final : public Statement
    {
    public:
        ScopeStatement(Token left_brace_token, Token right_brace_token, std::vector<const Statement*> statements)
            :Statement(NodeInfo{.tokenList = {left_brace_token}, .isValid = false, .lineNumber = left_brace_token.lineNumber}), 
            m_leftBraceToken(left_brace_token), m_rightBraceToken(right_brace_token), m_statements(std::move(statements))
        {
            for(const auto& statement: m_statements)
                addTokens(statement->getTokens());
            addToken(right_brace_token);
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            std::vector<const Node*> nodes;
            for(const auto& statement: m_statements)
                nodes.push_back(statement);
            
            return nodes;
        }

        inline const std::vector<const Statement*>& getStatements() const { return m_statements; }
        inline const Statement* getStatement(std::vector<const Statement*>::size_type index) const { return m_statements.at(index); }
    private:
        Token m_leftBraceToken, m_rightBraceToken;
        const std::vector<const Statement*> m_statements;
    };
}