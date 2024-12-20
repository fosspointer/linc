#pragma once
#include <linc/parser/Program.hpp>
#include <linc/system/ScopeStack.hpp>
#include <linc/system/StringStack.hpp>
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundBinaryExpression.hpp>
#include <linc/bound_tree/BoundUnaryExpression.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    template <typename T>
    class BoundNodeListClause;
    template <typename T>
    class NodeListClause;

    /// @brief Management class for 'registering' and scoping symbols.
    class BoundSymbols final
    {
    public:
        BoundSymbols();
        void clear();
        
        [[nodiscard]] std::unique_ptr<const class BoundDeclaration> find(const std::string& name, bool top_only = false) const;
        [[nodiscard]] bool push(std::unique_ptr<const class BoundDeclaration> symbol);

        [[nodiscard]] inline std::string findLabel(const std::string& name)
        {
            return m_labels.find(name);
        }

        inline void pushLabel(const std::string& name) { m_labels.push(name); }
        inline void popLabel() { m_labels.pop(); }
        inline void beginScope() { m_scopes.beginScope(); }
        inline void endScope() { m_scopes.endScope(); }

        [[nodiscard]] inline std::vector<const std::unique_ptr<const class BoundDeclaration>*> getSymbols() const
        {
            return m_scopes.getSymbols();
        } 
    private:
        ScopeStack<std::unique_ptr<const class BoundDeclaration>> m_scopes;
        StringStack m_labels;
    };

    /// @brief Class responsible for the binding stage of compilation.
    class Binder final
    {
    public:
        [[nodiscard]] inline std::vector<const std::unique_ptr<const class BoundDeclaration>*> getSymbols()
        {
            return m_boundDeclarations.getSymbols();
        }

        [[nodiscard]] class BoundProgram bindProgram(const Program* program);
        [[nodiscard]] std::unique_ptr<const class BoundNode> bindNode(const Node* node);
        [[nodiscard]] std::unique_ptr<const class BoundStatement> bindStatement(const class Statement* statement);
        [[nodiscard]] std::unique_ptr<const class BoundDeclaration> bindDeclaration(const class Declaration* expression);
        [[nodiscard]] std::unique_ptr<const class BoundExpression> bindExpression(const class Expression* expression);

        [[nodiscard]] inline auto find(const std::string& name){ return m_boundDeclarations.find(name); }

        inline void reset() { m_boundDeclarations.clear(); }
    private:
        static void reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::type left_type, Types::type right_type,
            const Token::Info& info);
        static void reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::type operand_type, const Token::Info& info);
        template <typename FROM, typename FUNC>
        [[nodiscard]] inline const auto bindNodeListClause(const class NodeListClause<FROM>* clause, FUNC bind_function)
        {
            using NodeType = typename std::remove_const<typename std::remove_reference<decltype(*(this->*bind_function)(std::declval<const FROM* const>()))>::type>::type;

            std::vector<std::unique_ptr<const NodeType>> values;
            values.reserve(clause->getList().size());
            for(const auto& value: clause->getList())
            {
                auto bound_value = Types::uniqueCast<const NodeType>((this->*bind_function)(value.node.get()));
                values.push_back(std::move(bound_value));
            }

            return std::make_unique<const BoundNodeListClause<NodeType>>(std::move(values), clause->getTokenInfo());
        }
        
        [[nodiscard]] const std::unique_ptr<const class BoundDeclarationStatement> bindDeclarationStatement(const class DeclarationStatement* statement);
        [[nodiscard]] const std::unique_ptr<const class BoundExpressionStatement> bindExpressionStatement(const class ExpressionStatement* statement);
        [[nodiscard]] const std::unique_ptr<const class BoundBlockExpression> bindBlockExpression(const class BlockExpression* statement);
        [[nodiscard]] const std::unique_ptr<const class BoundReturnStatement> bindReturnStatement(const class ReturnStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const class BoundContinueStatement> bindContinueStatement(const class ContinueStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const class BoundBreakStatement> bindBreakStatement(const class BreakStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const class BoundVariableDeclaration> bindVariableDeclaration(const class VariableDeclaration* declaration,
            bool is_argument = false);
        [[nodiscard]] const std::unique_ptr<const class BoundVariableDeclaration> bindDirectVariableDeclaration(const class DirectVariableDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const class BoundFunctionDeclaration> bindFunctionDeclaration(const class FunctionDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const class BoundExternalDeclaration> bindExternalDeclaration(const class ExternalDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const class BoundStructureDeclaration> bindStructureDeclaration(const class StructureDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const class BoundEnumerationDeclaration> bindEnumerationDeclaration(const class EnumerationDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const class BoundIdentifierExpression> bindIdentifierExpression(const class IdentifierExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundEnumeratorExpression> bindEnumeratorExpression(const class EnumeratorExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundTypeExpression> bindTypeExpression(const class TypeExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundIfExpression> bindIfExpression(const class IfExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundWhileExpression> bindWhileExpression(const class WhileExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundMatchExpression> bindMatchExpression(const class MatchExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundForExpression> bindForExpression(const class ForExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundFunctionCallExpression> bindFunctionCallExpression(const class CallExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundExternalCallExpression> bindExternalCallExpression(const class CallExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundLiteralExpression> bindLiteralExpression(const class LiteralExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundUnaryExpression> bindUnaryExpression(const class UnaryExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundBinaryExpression> bindBinaryExpression(const class BinaryExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundConversionExpression> bindConversionExpression(const class ConversionExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundArrayInitializerExpression> bindArrayInitializerExpression(const class ArrayInitializerExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundIndexExpression> bindIndexExpression(const class IndexExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundAccessExpression> bindAccessExpression(const class AccessExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundStructureInitializerExpression> bindStructureInitializerExpression(const class StructureInitializerExpression* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundMatchClause> bindMatchClause(const class MatchClause* expression);
        [[nodiscard]] const std::unique_ptr<const class BoundEnumeratorClause> bindEnumeratorClause(const class EnumeratorClause* expression);
        [[nodiscard]] BoundUnaryOperator::Kind bindUnaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundBinaryOperator::Kind bindBinaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundTypeExpression::BoundArraySpecifiers bindArraySpecifiers(const std::vector<TypeExpression::ArraySpecifier>& specifiers);

        BoundSymbols m_boundDeclarations;
        Types::u64 m_inLoop{}, m_inFunction{};
        std::stack<std::string> m_matchIdentifiers{};
        Types::type m_currentFunctionType{Types::voidType};
    };
}