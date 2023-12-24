#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <memory>

namespace linc
{
    class VariableDeclarationStatement final : public Statement
    {
    public:
        VariableDeclarationStatement(const Token& typename_identifier_token, const Token& assignment_operator_token,
            const std::optional<Token>& var_keyword_token, std::unique_ptr<const Expression> expression,
            std::unique_ptr<const IdentifierExpression> identifier_expression)
            :Statement(NodeInfo{.tokenList = {typename_identifier_token}, .lineNumber = typename_identifier_token.lineNumber}),
            m_typeNameIdentifierToken(typename_identifier_token), m_assignmentOperatorToken(assignment_operator_token),
            m_expression(std::move(expression)), m_identifierExpression(std::move(identifier_expression)), m_varKeywordToken(var_keyword_token)
        {
            if(m_typeNameIdentifierToken.type != Token::Type::Identifier)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Expected a typename identifier expression. Got '$' instead.",
                        Token::typeToString(m_typeNameIdentifierToken.type))
                });

            if(m_varKeywordToken.has_value() && m_varKeywordToken->type != Token::Type::KeywordVar)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = linc::Logger::format("Expected variability specifier keyword. Got '$' instead.",
                        Token::typeToString(m_varKeywordToken->type))
                });

            if(m_varKeywordToken.has_value())
                addToken(m_varKeywordToken.value());
            addToken(m_typeNameIdentifierToken);
            addTokens(m_identifierExpression->getTokens());
            addToken(m_assignmentOperatorToken);
            addTokens(m_expression->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_identifierExpression.get(), m_expression.get()};
        }

        virtual std::unique_ptr<const Statement> clone_const() const final override
        {
            std::unique_ptr<const IdentifierExpression> identifier_expression_clone = std::make_unique<const IdentifierExpression>(m_identifierExpression->getIdentifierToken());

            return std::make_unique<const VariableDeclarationStatement>(m_typeNameIdentifierToken, m_assignmentOperatorToken, m_varKeywordToken,
                std::move(m_expression->clone_const()), std::move(identifier_expression_clone));
        }

        inline const Token& getTypeNameIdentifierToken() const { return m_typeNameIdentifierToken; }
        inline const Token& getAssignmentOperatorToken() const { return m_assignmentOperatorToken; }
        inline const std::optional<Token>& getVariableKeywordToken() const { return m_varKeywordToken; }
        inline const Expression* const getExpression() const { return m_expression.get(); }
        inline const IdentifierExpression* const getIdentifierExpression() const { return m_identifierExpression.get(); }
    private:
        const Token m_typeNameIdentifierToken, m_assignmentOperatorToken;
        const std::optional<Token> m_varKeywordToken;
        const std::unique_ptr<const Expression> m_expression;
        const std::unique_ptr<const IdentifierExpression> m_identifierExpression;
    };
}