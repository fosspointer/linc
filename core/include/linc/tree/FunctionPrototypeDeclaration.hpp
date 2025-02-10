#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/NodeListClause.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class IdentifierExpression;
    class TypeExpression;
    class VariableDeclaration;
    class FunctionPrototypeDeclaration final : public Declaration
    {
    public:
        FunctionPrototypeDeclaration(const Token& function_specifier, const Token& type_specifier, const Token& left_parenthesis,
            const Token& right_parenenthesis, std::unique_ptr<const IdentifierExpression> identifier, std::unique_ptr<const TypeExpression> type,
            std::unique_ptr<const NodeListClause<VariableDeclaration>> arguments);
        ~FunctionPrototypeDeclaration();

        virtual std::unique_ptr<const Declaration> clone() const final override;
        inline const Token& getFunctionSpecifier() const { return m_functionSpecifier; }
        inline const Token& getTypeSpecifier() const { return m_typeSpecifier; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const TypeExpression* const getReturnType() const { return m_returnType? m_returnType.get(): nullptr; }
        inline const NodeListClause<VariableDeclaration>* getArguments() const { return m_arguments.get(); }
    private:
        const Token m_functionSpecifier, m_typeSpecifier, m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const TypeExpression> m_returnType;
        const std::unique_ptr<const NodeListClause<VariableDeclaration>> m_arguments;
    };
}