#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/TypeExpression.hpp>
#include <linc/tree/VariableDeclaration.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    class FunctionDeclaration final : public Declaration
    {
    public:
        FunctionDeclaration(const Token& function_specifier, const Token& type_specifier, const Token& left_parenthesis,
            const Token& right_parenenthesis, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const TypeExpression> type,
            std::unique_ptr<const NodeListClause<VariableDeclaration>> arguments, std::unique_ptr<const Expression> body)
            :Declaration(std::move(identifier), function_specifier.info), m_functionSpecifier(function_specifier), m_typeSpecifier(type_specifier),
            m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenenthesis), m_returnType(std::move(type)), m_arguments(std::move(arguments)),
            m_body(std::move(body))
        {
            LINC_NODE_ASSERT(m_functionSpecifier, Token::Type::KeywordFunction);
            LINC_NODE_ASSERT(m_leftParenthesis, Token::Type::ParenthesisLeft);
            LINC_NODE_ASSERT(m_rightParenthesis, Token::Type::ParenthesisRight);
            LINC_NODE_ASSERT(m_typeSpecifier, Token::Type::Colon);

            addToken(m_functionSpecifier);
            addTokens(m_identifier->getTokens());
            addToken(m_leftParenthesis);
            addTokens(m_arguments->getTokens());
            addTokens(std::vector<Token>{m_rightParenthesis, m_typeSpecifier});
            if(m_returnType)
                addTokens(m_returnType->getTokens());
            addTokens(m_body->getTokens());
        }

        virtual std::unique_ptr<const Declaration> clone() const final override
        {
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            auto type = m_returnType? Types::uniqueCast<const TypeExpression>(m_returnType->clone()): nullptr;

            return std::make_unique<const FunctionDeclaration>(
                m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis,
                std::move(identifier), std::move(type), m_arguments->clone(), m_body->clone());
        }

        inline const Token& getFunctionSpecifier() const { return m_functionSpecifier; }
        inline const Token& getTypeSpecifier() const { return m_typeSpecifier; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const TypeExpression* const getReturnType() const { return m_returnType.get(); }
        inline const NodeListClause<VariableDeclaration>* getArguments() const { return m_arguments.get(); }
        inline const Expression* const getBody() const { return m_body.get(); }
    private:
        const Token m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const TypeExpression> m_returnType;
        const std::unique_ptr<const NodeListClause<VariableDeclaration>> m_arguments;
        const std::unique_ptr<const Expression> m_body;
    };
}