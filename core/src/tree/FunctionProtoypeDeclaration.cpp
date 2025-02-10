#include <linc/tree/FunctionPrototypeDeclaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/TypeExpression.hpp>
#include <linc/tree/VariableDeclaration.hpp>

namespace linc
{
    FunctionPrototypeDeclaration::FunctionPrototypeDeclaration(const Token& function_specifier, const Token& type_specifier, const Token& left_parenthesis,
        const Token& right_parenenthesis, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const TypeExpression> type,
        std::unique_ptr<const NodeListClause<VariableDeclaration>> arguments)
        :Declaration(std::move(identifier), function_specifier.info), m_functionSpecifier(function_specifier), m_typeSpecifier(type_specifier),
        m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenenthesis), m_returnType(std::move(type)), m_arguments(std::move(arguments))
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
    }
    FunctionPrototypeDeclaration::~FunctionPrototypeDeclaration() = default;

    std::unique_ptr<const Declaration> FunctionPrototypeDeclaration::clone() const
    {
        auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
        auto type = m_returnType? Types::uniqueCast<const TypeExpression>(m_returnType->clone()): nullptr;

        return std::make_unique<const FunctionPrototypeDeclaration>(
            m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis,
            std::move(identifier), std::move(type), m_arguments->clone());
    }
}