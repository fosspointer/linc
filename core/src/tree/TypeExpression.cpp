#include <linc/tree/TypeExpression.hpp>
#include <linc/tree/LiteralExpression.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    TypeExpression::TypeExpression(const std::optional<Token>& mutability_Keyword, Root root, std::vector<ArraySpecifier> array_specifiers)
        :Expression(Token::Info{}), m_root(std::move(root)), m_mutabilityKeyword(mutability_Keyword),
        m_arraySpecifiers(std::move(array_specifiers))
    {
        handleTokens();
    }

    std::unique_ptr<const Expression> TypeExpression::clone() const
    {
        std::vector<ArraySpecifier> array_specifiers;
        array_specifiers.reserve(m_arraySpecifiers.size());

        for(const auto& specifier: m_arraySpecifiers)
        {
            auto count = specifier.count? specifier.count->clone(): nullptr;
            array_specifiers.push_back(ArraySpecifier{
                .leftBracket = specifier.leftBracket, .rightBracket = specifier.rightBracket,
                .count = Types::uniqueCast<const LiteralExpression>(std::move(count))
            });
        }

        if(auto function_root = std::get_if<FunctionRoot>(&m_root))
        {
            auto arguments = function_root->argumentTypes->clone();
            auto return_type = Types::uniqueCast<const TypeExpression>(function_root->returnType->clone());

            auto root = FunctionRoot{
                .functionKeyword = function_root->functionKeyword,
                .typeSpecifier = function_root->typeSpecifier, 
                .leftParenthesis = function_root->leftParenthesis,
                .rightParenthesis = function_root->rightParenthesis,
                .returnType = std::move(return_type),
                .argumentTypes = std::move(arguments)
                };
            
            return std::make_unique<const TypeExpression>(m_mutabilityKeyword, std::move(root), std::move(array_specifiers));
        }

        auto identifier = Types::uniqueCast<const IdentifierExpression>(std::get<0ul>(m_root)->clone());
        return std::make_unique<const TypeExpression>(m_mutabilityKeyword, std::move(identifier), std::move(array_specifiers));
    }

    void TypeExpression::handleTokens() const
    {
        if(m_mutabilityKeyword)
            addToken(*m_mutabilityKeyword);
        
        if(auto function_root = std::get_if<1ul>(&m_root))
        {
            addToken(function_root->functionKeyword);
            addToken(function_root->leftParenthesis);
            addTokens(function_root->argumentTypes->getTokens());
            addToken(function_root->rightParenthesis);
            addToken(function_root->typeSpecifier);
            addTokens(function_root->returnType->getTokens());
        }
        else addTokens(std::get<0ul>(m_root)->getTokens());

        for(const auto& specifier: m_arraySpecifiers)
            if(specifier.leftBracket && specifier.rightBracket)
            {
                addToken(*specifier.leftBracket);
                if(specifier.count)
                    addTokens(specifier.count->getTokens());
                addToken(*specifier.rightBracket);
            }
    }
}