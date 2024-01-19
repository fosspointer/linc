#pragma once
#include <linc/tree/Declaration.hpp>

#define LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(field_name, token_type) \
    if(field_name.type != token_type) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::AST, \
            .message = Logger::format("Expected '$'. Got '$' instead", \
                Token::typeToString(token_type), Token::typeToString(field_name.type))});

namespace linc
{
    class ArgumentDeclaration final : public Declaration
    {
    public:
        ArgumentDeclaration(const Token& typename_identifier_token, const Token& varname_identifier_token, 
            const std::optional<Token>& mutability_keyword_token, const std::optional<Token>& assignment_token,
            std::optional<std::unique_ptr<const Expression>> default_value_expression)
            :m_typenameIdentifierToken(typename_identifier_token), m_varnameIdentifierToken(varname_identifier_token),
            m_mutabilityKeywordToken(mutability_keyword_token), m_assignmentToken(assignment_token),
            m_defaultValueExpression(std::move(default_value_expression))
        {}

        inline const Token& getTypenameIdentifierToken() const { return m_typenameIdentifierToken; }
        inline const Token& getVarnameIdentifierToken() const { return m_varnameIdentifierToken; }
        inline const std::optional<Token>& getMutableToken() const { return m_mutabilityKeywordToken; }
        inline const std::optional<Token>& getAssignmentToken() const { return m_assignmentToken; }
        inline std::optional<const Expression*> getDefaultValueExpression() const
        {
            if(m_defaultValueExpression.has_value())
                return m_defaultValueExpression.value().get();
            else return std::nullopt;
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {};
        }

        virtual std::unique_ptr<const Declaration> clone_const() const final override
        {
            if(m_defaultValueExpression.has_value())
                return std::make_unique<ArgumentDeclaration>(m_typenameIdentifierToken, m_varnameIdentifierToken, m_mutabilityKeywordToken,
                    m_assignmentToken, std::move(m_defaultValueExpression.value()->clone_const()));
            else return std::make_unique<ArgumentDeclaration>(m_typenameIdentifierToken, m_varnameIdentifierToken, m_mutabilityKeywordToken,
                m_assignmentToken, std::nullopt);
        }
    private:
        Token m_typenameIdentifierToken, m_varnameIdentifierToken;
        std::optional<Token> m_mutabilityKeywordToken, m_assignmentToken;
        std::optional<std::unique_ptr<const Expression>> m_defaultValueExpression;
    };

    class FunctionDeclaration final : public Declaration
    {
    public:
        FunctionDeclaration(const Token& function_token, const Token& identifier_token, const Token& left_parenthesis_token,
            const Token& right_parenenthesis_token, const Token& type_operator_token, const Token& return_type_token, 
            std::vector<std::unique_ptr<const ArgumentDeclaration>> arguments, std::unique_ptr<const Statement> body)
            :m_functionToken(function_token), m_identifierToken(identifier_token), m_leftParenthesisToken(left_parenthesis_token),
            m_rightParenthesisToken(right_parenenthesis_token), m_typeOperatorToken(type_operator_token), 
            m_returnTypeToken(return_type_token), m_arguments(std::move(arguments)), m_body(std::move(body))
        {
            
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_functionToken, Token::Type::KeywordFunction);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_identifierToken, Token::Type::Identifier);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_leftParenthesisToken, Token::Type::ParenthesisLeft);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_rightParenthesisToken, Token::Type::ParenthesisRight);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_typeOperatorToken, Token::Type::OperatorType);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_returnTypeToken, Token::Type::Identifier);

            addTokens({m_functionToken, m_identifierToken, m_leftParenthesisToken});
            for(const auto& argument: m_arguments)
            {
                if(argument->getMutableToken().has_value())
                    addToken(argument->getMutableToken().value());
                
                addTokens({argument->getTypenameIdentifierToken(), argument->getVarnameIdentifierToken()});

                if(argument->getAssignmentToken().has_value() && argument->getDefaultValueExpression().has_value())
                {
                    addToken(argument->getAssignmentToken().value());
                    addTokens(argument->getDefaultValueExpression().value()->getTokens());
                }
            }
            addTokens({m_rightParenthesisToken, m_typeOperatorToken, m_returnTypeToken});
            addTokens(m_body->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_body.get()};
        }

        virtual std::unique_ptr<const Declaration> clone_const() const final override
        {
            std::vector<std::unique_ptr<const ArgumentDeclaration>> arguments;

            for(const auto& argument: m_arguments)
            {
                arguments.push_back(std::make_unique<ArgumentDeclaration>(
                    argument->getTypenameIdentifierToken(),
                    argument->getVarnameIdentifierToken(),
                    argument->getMutableToken(),
                    argument->getAssignmentToken(),
                    argument->getDefaultValueExpression().has_value()? std::move(argument->getDefaultValueExpression().value()->clone_const())
                        :std::optional<std::unique_ptr<const Expression>>{}
                ));
            }

            return std::make_unique<const FunctionDeclaration>(
                m_functionToken, m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken, m_typeOperatorToken, m_returnTypeToken,
                std::move(arguments), std::move(m_body->clone_const())
            );
        }

        inline const Token& getFunctionToken() const { return m_functionToken; }
        inline const Token& getIdentifierToken() const { return m_identifierToken; }
        inline const Token& getLeftParenthesisToken() const { return m_leftParenthesisToken; }
        inline const Token& getRightParenthesisToken() const { return m_rightParenthesisToken; }
        inline const Token& getTypeOperatorToken() const { return m_typeOperatorToken; }
        inline const Token& getReturnTypeToken() const { return m_returnTypeToken; }
        inline const std::vector<std::unique_ptr<const ArgumentDeclaration>>& getArguments() const { return m_arguments; }
        inline const Statement* const getBody() const { return m_body.get(); }
    private:
        const Token m_functionToken, m_identifierToken, m_leftParenthesisToken, m_rightParenthesisToken, m_typeOperatorToken, m_returnTypeToken;
        const std::vector<std::unique_ptr<const ArgumentDeclaration>> m_arguments;
        const std::unique_ptr<const Statement> m_body;
    };
}