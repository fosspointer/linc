#include <linc/tree/NamespaceDeclaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    NamespaceDeclaration::NamespaceDeclaration(const Token& namespace_keyword, const Token& left_brace, const Token& right_brace, std::unique_ptr<const IdentifierExpression> identifier,
        std::vector<std::unique_ptr<const Declaration>> declarations)
        :Declaration(std::move(identifier), AttributeMap{}), m_namespaceKeyword(namespace_keyword), m_leftBrace(left_brace), m_rightBrace(left_brace),
        m_declarations(std::move(declarations))
    {
        addToken(m_namespaceKeyword);
        addToken(m_leftBrace);
        for(const auto& declaration: m_declarations)
            addTokens(declaration->getTokens());
        addToken(m_rightBrace);
    }
    NamespaceDeclaration::~NamespaceDeclaration() = default;

    std::unique_ptr<const Declaration> NamespaceDeclaration::clone() const
    {
        auto identifier = Memory::uniqueCast<const IdentifierExpression>(m_identifier->clone());
        std::vector<std::unique_ptr<const Declaration>> declarations;
        declarations.reserve(m_declarations.size());
        for(const auto& declaration: m_declarations)
            declarations.push_back(declaration->clone());
        return std::make_unique<const NamespaceDeclaration>(m_namespaceKeyword, m_leftBrace, m_rightBrace, std::move(identifier), std::move(declarations));
    }
}