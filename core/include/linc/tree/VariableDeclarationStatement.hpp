#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    class VariableDeclarationStatement final : public Statement
    {
    public:
        VariableDeclarationStatement(Token typename_identifier_token, Token assignment_operator_token,
             const Expression* expression, const IdentifierExpression* identifier_expression)
            :Statement(NodeInfo{.tokenList = {typename_identifier_token}, 
                .isValid = expression->isValid() && identifier_expression->isValid() && typename_identifier_token.isValid()
                    && assignment_operator_token.isValid(), .lineNumber = typename_identifier_token.lineNumber}),
            m_typeNameIdentifierToken(typename_identifier_token), m_assignmentOperatorToken(assignment_operator_token),
            m_expression(expression), m_identifierExpression(identifier_expression)
        {
            if(typename_identifier_token.type != Token::Type::Identifier)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Expected a typename identifier expression. Got '$' instead.",
                        Token::typeToString(typename_identifier_token.type))
                });
            }

            addTokens(identifier_expression->getTokens());
            addToken(assignment_operator_token);
            addTokens(expression->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_identifierExpression, m_expression};
        }

        inline Token getTypeNameIdentifierToken() const { return m_typeNameIdentifierToken; }
        inline Token getAssignmentOperatorToken() const { return m_assignmentOperatorToken; }
        inline const Expression* getExpression() const { return m_expression; }
        inline const IdentifierExpression* getIdentifierExpression() const { return m_identifierExpression; }
    private:
        Token m_typeNameIdentifierToken, m_assignmentOperatorToken;
        const Expression* m_expression;
        const IdentifierExpression* m_identifierExpression;
    };
}