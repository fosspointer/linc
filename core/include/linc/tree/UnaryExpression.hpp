#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class UnaryExpression final : public Expression
    {
    public:
        UnaryExpression(const Token& operator_token, std::unique_ptr<const Expression> operand)
            :Expression(operator_token.info),
            m_operand(std::move(operand)), m_operatorToken(operator_token)
        {
            addToken(m_operatorToken);
            addTokens(m_operand->getTokens());

            if(!m_operatorToken.isUnaryOperator())
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                        .message = "Unary expression expected a unary operator"});
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const UnaryExpression>(m_operatorToken, m_operand->clone());
        }

        inline const Expression* const getOperand() const { return m_operand.get(); }
        inline const Expression* const getOperand() { return m_operand.get(); }

        inline const Token& getOperatorToken() const { return m_operatorToken; }
    private:
        const std::unique_ptr<const Expression> m_operand;
        const Token m_operatorToken;
    };
}