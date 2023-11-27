#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class ParenthesisExpression final : public Expression
    {
    public:
        ParenthesisExpression(const Token& left_parenthesis_token, const Token& right_parenthesis_token, Expression* expression)
            :Expression(NodeInfo{
                .tokenList = {left_parenthesis_token},
                .isValid = expression->isValid(), .lineNumber = left_parenthesis_token.lineNumber}),
            m_expression(expression), m_leftParenthesisToken(left_parenthesis_token), m_rightParenthesisToken(right_parenthesis_token)
        {
            addTokens(expression->getTokens());
            addToken(right_parenthesis_token);

            if(left_parenthesis_token.type != Token::Type::ParenthesisLeft)
            {
                setValid(false);
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error,
                    .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Parenthesis expression expected opening parenthesis, got '$' instead.", 
                        Token::typeToString(left_parenthesis_token.type))
                });
            }
            
            if(right_parenthesis_token.type != Token::Type::ParenthesisRight)
            {
                setValid(false);
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error,
                    .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Parenthesis expression expected opening parenthesis, got '$' instead.", 
                        Token::typeToString(right_parenthesis_token.type))
                });
            }
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_expression};
        }

        inline const Expression* getExpression() const { return m_expression; }
        inline const Expression* getExpression() { return m_expression; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesisToken; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesisToken; }
    private:
        const Expression* m_expression;
        Token m_leftParenthesisToken, m_rightParenthesisToken;
    };
}