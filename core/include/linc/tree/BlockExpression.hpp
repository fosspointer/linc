#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BlockExpression final : public Expression
    {
    public:
        BlockExpression(Token left_brace_token, Token right_brace_token, std::vector<std::unique_ptr<const Statement>> statements,
            std::unique_ptr<const Expression> tail)
            :Expression(NodeInfo{.tokenList = {left_brace_token}, .info = left_brace_token.info}), 
            m_leftBraceToken(left_brace_token), m_rightBraceToken(right_brace_token), m_statements(std::move(statements)),
            m_tail(std::move(tail))
        {
            for(const auto& statement: m_statements)
                addTokens(statement->getTokens());
            addToken(m_rightBraceToken);
        }

        inline const std::vector<std::unique_ptr<const Statement>>& getStatements() const { return m_statements; }
        inline const Statement* const getStatement(std::vector<const Statement*>::size_type index) const { return m_statements.at(index).get(); }
        inline const Expression* const getTail() const { return m_tail.get(); }
        
        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<std::unique_ptr<const Statement>> statements;
            statements.reserve(m_statements.size());

            for(const auto& statement: m_statements)
                statements.push_back(std::move(statement->clone()));

            auto tail = m_tail? m_tail->clone(): nullptr;
            return std::make_unique<const BlockExpression>(m_leftBraceToken, m_rightBraceToken, std::move(statements), std::move(tail));
        }
    private:
        const Token m_leftBraceToken, m_rightBraceToken;
        const std::vector<std::unique_ptr<const Statement>> m_statements;
        const std::unique_ptr<const Expression> m_tail;
    };
}