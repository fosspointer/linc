#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class UnaryExpression final : public Expression
    {
    public:
        UnaryExpression(const Token& operator_token, Expression* operand)
            :Expression({.isValid = true, .lineNumber = operator_token.lineNumber}),
            m_operand(operand), m_operator(operator_token)
        {
            addToken(operator_token);
            addTokens(m_operand->getTokens());

            if(!m_operator.isUnaryOperator())
            {
                setValid(false);
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                        .message = "Unary expression expected a unary operator"});
            }
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_operand};
        }

        inline const Expression* getOperand() const { return m_operand; }
        inline const Expression* getOperand() { return m_operand; }

        inline const Token& getOperatorToken() const { return m_operator; }
        inline Token& getOperatorToken() { return m_operator; }
    private:
        const Expression* m_operand;
        Token m_operator;
    };
}