#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/LiteralExpression.hpp>

namespace linc
{
    class TypeExpression final : public Expression
    {
    public:
        struct FunctionRoot final
        {
            Token functionKeyword, typeSpecifier, leftParenthesis, rightParenthesis;
            std::unique_ptr<const TypeExpression> returnType;
            std::unique_ptr<const NodeListClause<TypeExpression>> argumentTypes;
        };

        using Root = std::variant<std::unique_ptr<const IdentifierExpression>, FunctionRoot>;

        struct ArraySpecifier final
        {
            std::optional<const Token> leftBracket, rightBracket;
            std::unique_ptr<const LiteralExpression> count;
        };

        TypeExpression(const std::optional<Token>& mutability_Keyword, Root root, std::vector<ArraySpecifier> array_specifiers);
        [[nodiscard]] inline const Root& getRoot() const { return m_root; }
        [[nodiscard]] inline const std::optional<const Token>& getMutabilityKeyword() const { return m_mutabilityKeyword; }
        [[nodiscard]] inline const std::vector<ArraySpecifier>& getArraySpecifiers() const { return m_arraySpecifiers; }
        [[nodiscard]] inline const IdentifierExpression* getIfIdentifierRoot() const { if(auto identifier_root = std::get_if<0ul>(&m_root)) return identifier_root->get(); else return nullptr; }
        [[nodiscard]] inline const FunctionRoot* getIfFunctionRoot() const { if(auto function_root = std::get_if<1ul>(&m_root)) return function_root; else return nullptr; }

        virtual std::unique_ptr<const Expression> clone() const final override;
    private:
        void handleTokens() const;

        const Root m_root;
        const std::optional<const Token> m_mutabilityKeyword;
        const std::vector<ArraySpecifier> m_arraySpecifiers;
    };
}