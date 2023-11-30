#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class VariableDeclarationStatement final : public Statement
    {
    public:
        VariableDeclarationStatement(Token typename_identifier_token, Token assignment_operator_token,
            std::unique_ptr<const Expression> expression, std::unique_ptr<const IdentifierExpression> identifier_expression)
            :Statement(NodeInfo{.tokenList = {typename_identifier_token}, 
                .isValid = expression->isValid() && identifier_expression->isValid() && typename_identifier_token.isValid()
                    && assignment_operator_token.isValid(), .lineNumber = typename_identifier_token.lineNumber}),
            m_typeNameIdentifierToken(typename_identifier_token), m_assignmentOperatorToken(assignment_operator_token),
            m_expression(std::move(expression)), m_identifierExpression(std::move(identifier_expression))
        {
            if(m_typeNameIdentifierToken.type != Token::Type::Identifier)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Expected a typename identifier expression. Got '$' instead.",
                        Token::typeToString(m_typeNameIdentifierToken.type))
                });
            }

            addTokens(m_identifierExpression->getTokens());
            addToken(m_assignmentOperatorToken);
            addTokens(m_expression->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_identifierExpression.get(), m_expression.get()};
        }

        inline Token getTypeNameIdentifierToken() const { return m_typeNameIdentifierToken; }
        inline Token getAssignmentOperatorToken() const { return m_assignmentOperatorToken; }
        inline const Expression* getExpression() const { return m_expression.get(); }
        inline const IdentifierExpression* getIdentifierExpression() const { return m_identifierExpression.get(); }
    private:
        Token m_typeNameIdentifierToken, m_assignmentOperatorToken;
        std::unique_ptr<const Expression> m_expression;
        std::unique_ptr<const IdentifierExpression> m_identifierExpression;
    };
}