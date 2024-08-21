#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>
#include <linc/parser/Program.hpp>

namespace linc
{
    /// @brief Management class for 'registering' and scoping symbols.
    class BoundSymbols final
    {
    public:
        BoundSymbols();
        struct Label final
        {
            Types::u64 blockIndex, scope;
            bool isLoop;
        };

        /// @brief Scope management class.
        class Scope final
        {
        public:
            /// @brief Default constructor
            Scope() = default;
            Scope(const Scope& scope);
            Scope(Scope&& scope);

            [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() const { return m_symbols; }
            [[nodiscard]] inline std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() { return m_symbols; }
        private:
            std::vector<std::unique_ptr<const BoundDeclaration>> m_symbols;
        };

        void clear();

        [[nodiscard]] inline std::vector<std::unique_ptr<const BoundDeclaration>>::const_iterator end() const
        {
            return m_scopes.top().getSymbols().end();
        }

        [[nodiscard]] inline std::unordered_map<std::string, Label>::const_iterator labelEnd() const
        {
            return m_labels.top().end();
        }

        [[nodiscard]] std::vector<std::unique_ptr<const BoundDeclaration>>::const_iterator find(const std::string& name) const;
        [[nodiscard]] bool push(std::unique_ptr<const BoundDeclaration> symbol);

        [[nodiscard]] inline std::unordered_map<std::string, Label>::const_iterator findLabel(const std::string& name)
        {
            return m_labels.top().find(name);
        }

        bool pushLabel(const std::string& name, Types::u64 block_index, Types::u64 scope, bool is_loop);
        void beginScope();
        void endScope();
        inline Types::u64 getScope() const
        {
            return m_scopes.size() - 1ul;
        }

        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() const
        {
            return m_scopes.top().getSymbols();
        } 
    private:
        std::stack<Scope> m_scopes;
        std::stack<std::unordered_map<std::string, Label>> m_labels;
    };

    /// @brief Class responsible for the binding stage of compilation.
    class Binder final
    {
    public:
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols()
        {
            return m_boundDeclarations.getSymbols();
        }

        [[nodiscard]] BoundProgram bind(const Program* program);
        [[nodiscard]] std::unique_ptr<const BoundNode> bindNode(const Node* node);
        [[nodiscard]] std::unique_ptr<const BoundStatement> bindStatement(const Statement* statement);
        [[nodiscard]] std::unique_ptr<const BoundDeclaration> bindDeclaration(const Declaration* expression);
        [[nodiscard]] std::unique_ptr<const BoundExpression> bindExpression(const Expression* expression);

        [[nodiscard]] inline auto find(const std::string& name){ return m_boundDeclarations.find(name); }
        [[nodiscard]] inline auto end(){ return m_boundDeclarations.end(); }

        inline void reset() { m_boundDeclarations.clear(); }
    private:
        static void reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::type left_type, Types::type right_type,
            const Token::Info& info);
        static void reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::type operand_type, const Token::Info& info);
        [[nodiscard]] const std::unique_ptr<const BoundDeclarationStatement> bindDeclarationStatement(const DeclarationStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundExpressionStatement> bindExpressionStatement(const ExpressionStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundBlockExpression> bindBlockExpression(const BlockExpression* statement);
        [[nodiscard]] const std::unique_ptr<const BoundLabelStatement> bindLabelStatement(const LabelStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundJumpStatement> bindJumpStatement(const JumpStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundReturnStatement> bindReturnStatement(const ReturnStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const BoundContinueStatement> bindContinueStatement(const ContinueStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const BoundBreakStatement> bindBreakStatement(const BreakStatement* statement);        
        [[nodiscard]] const std::unique_ptr<const BoundVariableDeclaration> bindVariableDeclaration(const VariableDeclaration* declaration,
            bool is_argument = false);
        [[nodiscard]] const std::unique_ptr<const BoundVariableDeclaration> bindDirectVariableDeclaration(const DirectVariableDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundFunctionDeclaration> bindFunctionDeclaration(const FunctionDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundExternalDeclaration> bindExternalDeclaration(const ExternalDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundStructureDeclaration> bindStructureDeclaration(const StructureDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundIdentifierExpression> bindIdentifierExpression(const IdentifierExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundTypeExpression> bindTypeExpression(const TypeExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundIfExpression> bindIfExpression(const IfExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundWhileExpression> bindWhileExpression(const WhileExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundForExpression> bindForExpression(const ForExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundFunctionCallExpression> bindFunctionCallExpression(const CallExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExternalCallExpression> bindExternalCallExpression(const CallExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundLiteralExpression> bindLiteralExpression(const LiteralExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundUnaryExpression> bindUnaryExpression(const UnaryExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundBinaryExpression> bindBinaryExpression(const BinaryExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundConversionExpression> bindConversionExpression(const ConversionExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundArrayInitializerExpression> bindArrayInitializerExpression(const ArrayInitializerExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundIndexExpression> bindIndexExpression(const IndexExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundAccessExpression> bindAccessExpression(const AccessExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundStructureInitializerExpression> bindStructureInitializerExpression(const StructureInitializerExpression* expression);
        [[nodiscard]] BoundUnaryOperator::Kind bindUnaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundBinaryOperator::Kind bindBinaryOperatorKind(Token::Type token_type);
        [[nodiscard]] BoundTypeExpression::BoundArraySpecifiers bindArraySpecifiers(const std::vector<TypeExpression::ArraySpecifier>& specifiers);

        BoundSymbols m_boundDeclarations;
        Types::u64 m_blockIndex{-1ul}, m_labelIdentifierIndex{-1ul};
        Types::type m_currentFunctionType{Types::voidType};
        bool m_inLoop, m_inFunction;
    };
}