#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BlockExpression final : public Expression
    {
    public:
        BlockExpression(const Token& left_brace, const Token& right_brace, std::vector<std::unique_ptr<const Statement>> statements,
            std::unique_ptr<const Expression> tail)
            :Expression(NodeInfo{.tokenList = {left_brace}, .info = left_brace.info}), 
            m_leftBrace(left_brace), m_rightBrace(right_brace), m_statements(std::move(statements)),
            m_tail(std::move(tail))
        {
            for(const auto& statement: m_statements)
                addTokens(statement->getTokens());
            addToken(m_rightBrace);
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
            return std::make_unique<const BlockExpression>(m_leftBrace, m_rightBrace, std::move(statements), std::move(tail));
        }
    private:
        const Token m_leftBrace, m_rightBrace;
        const std::vector<std::unique_ptr<const Statement>> m_statements;
        const std::unique_ptr<const Expression> m_tail;
    };
}