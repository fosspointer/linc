#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/TypeExpression.hpp>
#include <linc/tree/VariableDeclaration.hpp>

#define LINC_EXTERNAL_DECLARATION_TOKEN_TYPE_ASSERT(field_name, token_type) \
    if(field_name.type != token_type) \
        Reporting::push(Reporting::Report{ \
            .type = Reporting::Type::Error, .stage = Reporting::Stage::AST, \
            .message = Logger::format("Expected '$'. Got '$' instead", \
                Token::typeToString(token_type), Token::typeToString(field_name.type))});

namespace linc
{
    class ExternalDeclaration final : public Declaration
    {
    public:
        ExternalDeclaration(const Token& external_keyword, const Token& left_parenthesis, const Token& right_parenthesis, const Token& type_specifier,
            std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const TypeExpression> type,
            std::vector<std::unique_ptr<const TypeExpression>> arguments)
            :Declaration(external_keyword.info), m_externalKeyword(external_keyword), m_leftParenthesis(left_parenthesis),
            m_rightParenthesis(right_parenthesis), m_typeSpecifier(type_specifier), m_identifier(std::move(identifier)), m_actualType(std::move(type)),
            m_arguments(std::move(arguments)) 
        {
            LINC_EXTERNAL_DECLARATION_TOKEN_TYPE_ASSERT(m_externalKeyword, Token::Type::KeywordExternal);
            LINC_EXTERNAL_DECLARATION_TOKEN_TYPE_ASSERT(m_leftParenthesis, Token::Type::ParenthesisLeft);
            LINC_EXTERNAL_DECLARATION_TOKEN_TYPE_ASSERT(m_rightParenthesis, Token::Type::ParenthesisRight);
            LINC_EXTERNAL_DECLARATION_TOKEN_TYPE_ASSERT(m_typeSpecifier, Token::Type::Colon);
            
            addTokens(std::vector<Token>{m_externalKeyword, m_leftParenthesis});

            for(const auto& argument: m_arguments)
                addTokens(argument->getTokens());
            
            addTokens(std::vector<Token>{m_rightParenthesis, m_typeSpecifier});
            addTokens(m_actualType->getTokens());
        }

        virtual std::unique_ptr<const Declaration> clone() const final override
        {
            std::vector<std::unique_ptr<const TypeExpression>> arguments;

            for(const auto& argument: m_arguments)
            {
                auto type = Types::unique_cast<const TypeExpression>(argument->clone());
                arguments.push_back(std::move(type));
            }

            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->clone());
            auto type = Types::unique_cast<const TypeExpression>(m_actualType->clone());

            return std::make_unique<const ExternalDeclaration>(
                m_externalKeyword, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis,
                std::move(identifier), std::move(type), std::move(arguments));
        }

        inline const Token& getExternalKeyword() const { return m_externalKeyword; }
        inline const Token& getTypeSpecifier() const { return m_typeSpecifier; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const TypeExpression* const getActualType() const { return m_actualType.get(); }
        inline const std::vector<std::unique_ptr<const TypeExpression>>& getArguments() const { return m_arguments; }
    private:
        const Token m_externalKeyword, m_leftParenthesis, m_rightParenthesis, m_typeSpecifier;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::unique_ptr<const TypeExpression> m_actualType;
        const std::vector<std::unique_ptr<const TypeExpression>> m_arguments;
    };
}