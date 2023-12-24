#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ParenthesisExpression final : public Expression
    {
    public:
        ParenthesisExpression(const Token& left_parenthesis_token, const Token& right_parenthesis_token, std::unique_ptr<const Expression> expression)
            :Expression(NodeInfo{
                .tokenList = {left_parenthesis_token},
                .lineNumber = left_parenthesis_token.lineNumber}),
            m_expression(std::move(expression)), m_leftParenthesisToken(left_parenthesis_token), m_rightParenthesisToken(right_parenthesis_token)
        {
            addTokens(m_expression->getTokens());
            addToken(m_rightParenthesisToken);

            if(m_leftParenthesisToken.type != Token::Type::ParenthesisLeft)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error,
                    .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Parenthesis expression expected opening parenthesis, got '$' instead.", 
                        Token::typeToString(m_leftParenthesisToken.type))
                });
            
            if(m_rightParenthesisToken.type != Token::Type::ParenthesisRight)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error,
                    .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Parenthesis expression expected opening parenthesis, got '$' instead.", 
                        Token::typeToString(m_rightParenthesisToken.type))
                });
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_expression.get()};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const ParenthesisExpression>(m_leftParenthesisToken, m_rightParenthesisToken, std::move(m_expression->clone_const()));
        }

        inline const Expression* const getExpression() const { return m_expression.get(); }
        inline const Expression* const getExpression() { return m_expression.get(); }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesisToken; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesisToken; }
    private:
        const std::unique_ptr<const Expression> m_expression;
        const Token m_leftParenthesisToken, m_rightParenthesisToken;
    };
}