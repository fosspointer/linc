#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class UnaryExpression final : public Expression
    {
    public:
        UnaryExpression(const Token& operator_token, std::unique_ptr<const Expression> operand)
            :Expression({.isValid = true, .lineNumber = operator_token.lineNumber}),
            m_operand(std::move(operand)), m_operatorToken(operator_token)
        {
            addToken(m_operatorToken);
            addTokens(m_operand->getTokens());

            if(!m_operatorToken.isUnaryOperator())
            {
                setValid(false);
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                        .message = "Unary expression expected a unary operator"});
            }
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_operand.get()};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const UnaryExpression>(m_operatorToken, std::move(m_operand->clone_const()));
        }

        inline const Expression* getOperand() const { return m_operand.get(); }
        inline const Expression* getOperand() { return m_operand.get(); }

        inline const Token& getOperatorToken() const { return m_operatorToken; }
        inline Token& getOperatorToken() { return m_operatorToken; }
    private:
        std::unique_ptr<const Expression> m_operand;
        Token m_operatorToken;
    };
}