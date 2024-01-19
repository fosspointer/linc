#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class VariableDeclaration final : public Declaration
    {
    public:
        VariableDeclaration(const Token& typename_identifier_token, const Token& assignment_operator_token,
            const std::optional<Token>& mutability_keyword_token, std::unique_ptr<const Expression> expression,
            std::unique_ptr<const IdentifierExpression> identifier_expression)
            :Declaration(NodeInfo{.tokenList = {typename_identifier_token}, .lineNumber = typename_identifier_token.lineNumber}),
            m_typeNameIdentifierToken(typename_identifier_token), m_assignmentOperatorToken(assignment_operator_token),
            m_expression(std::move(expression)), m_identifierExpression(std::move(identifier_expression)), m_mutabilityKeywordToken(mutability_keyword_token)
        {
            if(m_typeNameIdentifierToken.type != Token::Type::Identifier)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = Logger::format("Expected a typename identifier expression. Got '$' instead.",
                        Token::typeToString(m_typeNameIdentifierToken.type))});

            if(m_mutabilityKeywordToken.has_value() && m_mutabilityKeywordToken->type != Token::Type::KeywordMutability)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::AST,
                    .message = Logger::format("Expected variability specifier keyword. Got '$' instead.",
                        Token::typeToString(m_mutabilityKeywordToken->type))});

            if(m_mutabilityKeywordToken.has_value())
                addToken(m_mutabilityKeywordToken.value());
            addToken(m_typeNameIdentifierToken);
            addTokens(m_identifierExpression->getTokens());
            addToken(m_assignmentOperatorToken);
            addTokens(m_expression->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_identifierExpression.get(), m_expression.get()};
        }

        virtual std::unique_ptr<const Declaration> clone_const() const final override
        {
            std::unique_ptr<const IdentifierExpression> identifier_expression_clone = std::make_unique<const IdentifierExpression>(
                m_identifierExpression->getIdentifierToken());

            return std::make_unique<const VariableDeclaration>(m_typeNameIdentifierToken, m_assignmentOperatorToken, m_mutabilityKeywordToken,
                std::move(m_expression->clone_const()), std::move(identifier_expression_clone));
        }

        inline const Token& getTypeNameIdentifierToken() const { return m_typeNameIdentifierToken; }
        inline const Token& getAssignmentOperatorToken() const { return m_assignmentOperatorToken; }
        inline const std::optional<Token>& getMutableKeywordToken() const { return m_mutabilityKeywordToken; }
        inline const Expression* const getExpression() const { return m_expression.get(); }
        inline const IdentifierExpression* const getIdentifierExpression() const { return m_identifierExpression.get(); }
    private:
        const Token m_typeNameIdentifierToken, m_assignmentOperatorToken;
        const std::optional<Token> m_mutabilityKeywordToken;
        const std::unique_ptr<const Expression> m_expression;
        const std::unique_ptr<const IdentifierExpression> m_identifierExpression;
    };
}