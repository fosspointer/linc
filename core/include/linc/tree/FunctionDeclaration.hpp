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
    class FunctionDeclaration final : public Declaration
    {
    public:
        FunctionDeclaration(const Token& function_specifier, const Token& type_specifier, const Token& left_parenthesis,
            const Token& right_parenenthesis, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const TypeExpression> type, 
            std::vector<std::unique_ptr<const VariableDeclaration>> arguments, std::unique_ptr<const Statement> body)
            :m_functionSpecifier(function_specifier), m_typeSpecifier(type_specifier), m_leftParenthesis(left_parenthesis),
            m_rightParenthesis(right_parenenthesis), m_identifier(std::move(identifier)), m_returnType(std::move(type)), 
            m_arguments(std::move(arguments)), m_body(std::move(body))
        {
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_functionSpecifier, Token::Type::KeywordFunction);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_leftParenthesis, Token::Type::ParenthesisLeft);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_rightParenthesis, Token::Type::ParenthesisRight);
            LINC_FUNCTION_DECLARATION_TOKEN_TYPE_ASSERT(m_typeSpecifier, Token::Type::TypeSpecifier);

            addTokens(std::vector<Token>{m_functionSpecifier, m_leftParenthesis});
            
            for(const auto& argument: m_arguments)
                addTokens(argument->getTokens());

            addTokens(std::vector<Token>{m_rightParenthesis, m_typeSpecifier});
            addTokens(m_returnType->getTokens());
            addTokens(m_body->getTokens());
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {m_body.get()};
        }

        virtual std::unique_ptr<const Declaration> cloneConst() const final override
        {
            std::vector<std::unique_ptr<const VariableDeclaration>> arguments;

            for(const auto& argument: m_arguments)
            {
                auto type = Types::unique_cast<const TypeExpression>(argument->getType()->cloneConst());
                auto identifier = Types::unique_cast<const IdentifierExpression>(argument->getIdentifier()->cloneConst());
                
                arguments.push_back(std::make_unique<const VariableDeclaration>(
                    argument->getTypeSpecifier(), std::move(type), std::move(identifier), argument->getDefaultValue()
                ));
            }

            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->cloneConst());
            auto type = Types::unique_cast<const TypeExpression>(m_returnType->cloneConst());

            return std::make_unique<const FunctionDeclaration>(
                m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis,
                std::move(identifier), std::move(type), std::move(arguments), m_body->cloneConst());
        }

        inline const Token& getFunctionSpecifier() const { return m_functionSpecifier; }
        inline const Token& getTypeSpecifier() const { return m_typeSpecifier; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const TypeExpression* const getReturnType() const { return m_returnType.get(); }
        inline const std::vector<std::unique_ptr<const VariableDeclaration>>& getArguments() const { return m_arguments; }
        inline const Statement* const getBody() const { return m_body.get(); }
    private:
        const Token m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const TypeExpression> m_returnType;
        const std::vector<std::unique_ptr<const VariableDeclaration>> m_arguments;
        const std::unique_ptr<const Statement> m_body;
    };
}