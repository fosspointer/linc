#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class BinaryExpression final : public Expression
    {
    public:
        BinaryExpression(const Token& operator_token, std::unique_ptr<const Expression> left, std::unique_ptr<const Expression> right)
            :Expression({.lineNumber = operator_token.lineNumber}),
            m_left(std::move(left)), m_right(std::move(right)), m_operatorToken(operator_token)
        {
            addTokens(m_left->getTokens());
            addToken(m_operatorToken);
            addTokens(m_right->getTokens());

            if(!m_operatorToken.isBinaryOperator())
            {
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                        .message = "Binary expression expected a binary operator"});
            }
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_left.get(), m_right.get()};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const BinaryExpression>(m_operatorToken, std::move(m_left->clone_const()), std::move(m_right->clone_const()));
        }

        inline const Expression* const getLeft() const { return m_left.get(); }
        inline const Expression* const getRight() const { return m_right.get(); }

        inline const Token& getOperatorToken() const { return m_operatorToken; }
    private:
        const std::unique_ptr<const Expression> m_left, m_right;
        const Token m_operatorToken;
    };
}