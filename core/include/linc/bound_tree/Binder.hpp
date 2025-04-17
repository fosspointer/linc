#pragma once
#include <linc/parser/Program.hpp>
#include <linc/system/ScopeStack.hpp>
#include <linc/system/StringStack.hpp>
#include <linc/system/Types.hpp>
#include <linc/system/Memory.hpp>
#include <linc/bound_tree/BoundBinaryExpression.hpp>
#include <linc/bound_tree/BoundUnaryExpression.hpp>
#include <linc/bound_tree/BoundTypeExpression.hpp>
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    template <typename T>
    class BoundNodeListClause;
    template <typename T>
    class NodeListClause;
    template <typename FIRST, typename SECOND>
    class VariantClause;
    template <typename FIRST, typename SECOND>
    class BoundVariantClause;

    /// @brief Management class for 'registering' and scoping symbols.
    class BoundSymbols final
    {
    public:
        BoundSymbols();
        void clear();
        
        [[nodiscard]] std::unique_ptr<const class BoundDeclaration> find(const std::string& name, bool top_only = false) const;
        [[nodiscard]] bool push(std::unique_ptr<const class BoundDeclaration> symbol);
        [[nodiscard]] std::size_t getScopeIndex() const { return m_scopes.getSize(); }
        void update(std::unique_ptr<const class BoundDeclaration> symbol);
        inline void appendWith(const std::string& name, std::unique_ptr<const class BoundDeclaration> symbol)
        {
            auto symbol_name = symbol->getName();
            m_scopes.appendWith(name, std::move(symbol_name), std::move(symbol));
        }

        [[nodiscard]] inline std::string findLabel(const std::string& name)
        {
            return m_labels.find(name);
        }

        inline void pushLabel(const std::string& name) { m_labels.push(name); }
        inline void popLabel() { m_labels.pop(); }
        inline void beginScope() { m_scopes.beginScope(); }
        inline void endScope() { m_scopes.endScope(); }
        inline void namespacePush(const std::string& item) { m_namespace.push_back(item); }
        inline void namespacePop() { m_namespace.pop_back(); }

        [[nodiscard]] inline std::vector<const std::unique_ptr<const class BoundDeclaration>*> getSymbols() const
        {
            return m_scopes.getSymbols();
        }

        [[nodiscard]] inline const std::vector<std::string>& getNamespace() const
        {
            return m_namespace;
        }

        [[nodiscard]] std::string getNamespaceString() const
        {
            std::string actual_namespace;
            for(auto it = m_namespace.begin(); it != m_namespace.end(); ++it)
                actual_namespace += *it + "::";
            return actual_namespace;
        }
    private:
        std::vector<std::string> m_namespace;
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

        [[nodiscard]] struct BoundProgram bindProgram(const Program* program);
        [[nodiscard]] std::unique_ptr<const class BoundNode> bindNode(const Node* node);
        [[nodiscard]] std::unique_ptr<const class BoundStatement> bindStatement(const class Statement* statement);
        [[nodiscard]] std::unique_ptr<const class BoundDeclaration> bindDeclaration(const class Declaration* expression);
        [[nodiscard]] std::unique_ptr<const class BoundExpression> bindExpression(const class Expression* expression);

        [[nodiscard]] inline auto find(const std::string& name){ return m_boundDeclarations.find(name); }
        [[nodiscard]] inline const std::vector<std::unordered_map<std::string, std::unique_ptr<const BoundDeclaration>>>& getGenericInstanceMaps() const { return m_genericInstanceMaps; }

        inline void reset() { m_boundDeclarations.clear(); }
    private:
        template <typename FROM, typename FUNC>
        [[nodiscard]] inline auto bindNodeListClause(const class NodeListClause<FROM>* clause, FUNC bind_function)
        {
            using NodeType = typename std::remove_const<typename std::remove_reference<decltype(*(this->*bind_function)(std::declval<const FROM* const>()))>::type>::type;

            std::vector<std::unique_ptr<const NodeType>> values;
            values.reserve(clause->getList().size());
            for(const auto& value: clause->getList())
            {
                auto bound_value = Memory::uniqueCast<const NodeType>((this->*bind_function)(value.node.get()));
                values.push_back(std::move(bound_value));
            }

            return std::make_unique<const BoundNodeListClause<NodeType>>(std::move(values), clause->getTokenInfo());
        }
        
        [[nodiscard]] std::unique_ptr<const class BoundDeclarationStatement> bindDeclarationStatement(const class DeclarationStatement* statement);
        [[nodiscard]] std::unique_ptr<const class BoundExpressionStatement> bindExpressionStatement(const class ExpressionStatement* statement);
        [[nodiscard]] std::unique_ptr<const class BoundBlockExpression> bindBlockExpression(const class BlockExpression* statement);
        [[nodiscard]] std::unique_ptr<const class BoundReturnStatement> bindReturnStatement(const class ReturnStatement* statement);        
        [[nodiscard]] std::unique_ptr<const class BoundContinueStatement> bindContinueStatement(const class ContinueStatement* statement);        
        [[nodiscard]] std::unique_ptr<const class BoundBreakStatement> bindBreakStatement(const class BreakStatement* statement);        
        [[nodiscard]] std::unique_ptr<const class BoundVariableDeclaration> bindVariableDeclaration(const class VariableDeclaration* declaration,
            bool is_argument = false);
        [[nodiscard]] std::unique_ptr<const class BoundVariableDeclaration> bindDirectVariableDeclaration(const class DirectVariableDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundFunctionPrototypeDeclaration> bindFunctionPrototypeDeclaration(const class FunctionPrototypeDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundFunctionDeclaration> bindFunctionDeclaration(const class FunctionDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundExternalDeclaration> bindExternalDeclaration(const class ExternalDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundStructureDeclaration> bindStructureDeclaration(const class StructureDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundEnumerationDeclaration> bindEnumerationDeclaration(const class EnumerationDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundAliasDeclaration> bindAliasDeclaration(const class AliasDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundGenericDeclaration> bindGenericDeclaration(const class GenericDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundNamespaceDeclaration> bindNamespaceDeclaration(const class NamespaceDeclaration* declaration);
        [[nodiscard]] std::unique_ptr<const class BoundIdentifierExpression> bindIdentifierExpression(const class IdentifierExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundEnumeratorExpression> bindEnumeratorExpression(const class EnumeratorExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundTypeExpression> bindTypeExpression(const class TypeExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundIfExpression> bindIfExpression(const class IfExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundWhileExpression> bindWhileExpression(const class WhileExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundMatchExpression> bindMatchExpression(const class MatchExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundForExpression> bindForExpression(const class ForExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundFunctionCallExpression> bindFunctionCallExpression(const class CallExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundExternalCallExpression> bindExternalCallExpression(const class CallExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundLiteralExpression> bindLiteralExpression(const class LiteralExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundUnaryExpression> bindUnaryExpression(const class UnaryExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundBinaryExpression> bindBinaryExpression(const class BinaryExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundConversionExpression> bindConversionExpression(const class ConversionExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundArrayInitializerExpression> bindArrayInitializerExpression(const class ArrayInitializerExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundIndexExpression> bindIndexExpression(const class IndexExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundExpression> bindAccessExpression(const class AccessExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundStructureInitializerExpression> bindStructureInitializerExpression(const class StructureInitializerExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundRangeExpression> bindRangeExpression(const class RangeExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundDefaultExpression> bindDefaultExpression(const class DefaultExpression* expression);
        [[nodiscard]] std::unique_ptr<const class BoundMatchClause> bindMatchClause(const class MatchClause* clause);
        [[nodiscard]] std::unique_ptr<const class BoundEnumeratorClause> bindEnumeratorClause(const class EnumeratorClause* clause);
        [[nodiscard]] std::unique_ptr<const class BoundVariantClause<class BoundLegacyForClause, class BoundRangedForClause>> bindForClause(const class VariantClause<class LegacyForClause, class RangedForClause>* clause);
        [[nodiscard]] std::string bindGenericClause(const class GenericClause* clause, const class BoundGenericDeclaration* declaration, const Token::Info& info);
        [[nodiscard]] BoundUnaryOperator::Kind bindUnaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundBinaryOperator::Kind bindBinaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundTypeExpression::BoundArraySpecifiers bindArraySpecifiers(const std::vector<TypeExpression::ArraySpecifier>& specifiers);
        [[nodiscard]] BoundDeclaration::DeprecatedMessage getDeprecatedMessage(const Declaration::AttributeMap& attributes, const std::string& symbol_name);
        void reportDeprecated(const std::string& identifier, std::string_view reason, const Token::Info& token_info);

        BoundSymbols m_boundDeclarations;
        Types::u64 m_inLoop{};
        std::stack<Types::type> m_functionReturnTypes;
        std::stack<std::string> m_matchIdentifiers;
        std::vector<std::unordered_map<std::string, std::unique_ptr<const BoundDeclaration>>> m_genericInstanceMaps;
        std::vector<std::string> m_deprecatedMessages;
    };
}