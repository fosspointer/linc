#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class BinaryExpression final : public Expression
    {
    public:
        BinaryExpression(const Token& operator_token, Expression* left, Expression* right)
            :Expression({.isValid = left->isValid() && right->isValid(), .lineNumber = operator_token.lineNumber}),
            m_left(left), m_right(right), m_operator(operator_token)
        {
            addTokens(left->getTokens());
            addToken(operator_token);
            addTokens(right->getTokens());

            if(!m_operator.isBinaryOperator())
            {
                setValid(false);
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                        .message = "Binary expression expected a binary operator"});
            }
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_left, m_right};
        }

        inline const Expression* getLeft() const { return m_left; }
        inline const Expression* getRight() const { return m_right; }

        inline const Token& getOperatorToken() const { return m_operator; }
        inline Token& getOperatorToken() { return m_operator; }
    private:
        const Expression* m_left, *m_right;
        Token m_operator;
    };
}