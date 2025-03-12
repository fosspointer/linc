#pragma once
#include <linc/tree/Declaration.hpp>

namespace linc
{
    class IdentifierExpression;
    class NamespaceDeclaration final : public Declaration
    {
    public:
        NamespaceDeclaration(const Token& namespace_keyword, const Token& left_brace, const Token& right_brace, std::unique_ptr<const IdentifierExpression> identifier,
            std::vector<std::unique_ptr<const Declaration>> declarations);
        ~NamespaceDeclaration();

        [[nodiscard]] inline const Token& getNamespaceKeyword() const { return m_namespaceKeyword; }
        [[nodiscard]] inline const Token& getBraceLeft() const { return m_leftBrace; }
        [[nodiscard]] inline const Token& getBraceRight() const { return m_rightBrace; }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const Declaration>>& getDeclarations() const { return m_declarations; }

        virtual std::unique_ptr<const Declaration> clone() const final override;
    private:
        const Token m_namespaceKeyword, m_leftBrace, m_rightBrace;
        const std::vector<std::unique_ptr<const Declaration>> m_declarations;
    };
}