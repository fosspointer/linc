#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>
#include <linc/parser/Program.hpp>

namespace linc
{
    class BoundDeclarations final
    {
    public:
        BoundDeclarations()
        {
            beginScope();
        }

        class Scope final
        {
        public:
            Scope() = default;

            Scope(const Scope& scope)
            {
                for(const auto& symbol: scope.m_symbols)
                    m_symbols.push_back(std::move(symbol->clone_const()));
            }

            Scope(Scope&& scope)
            {
                for(auto& symbol: scope.m_symbols)
                    m_symbols.push_back(std::move(symbol));
            }

            const std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() const { return m_symbols; }
            std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() { return m_symbols; }
        private:
            std::vector<std::unique_ptr<const BoundDeclaration>> m_symbols;
        };

        inline void clear()
        {
            while(!m_scopes.empty())
                m_scopes.pop();
            
            beginScope();
        }

        [[nodiscard]] inline std::vector<std::unique_ptr<const BoundDeclaration>>::const_iterator end() const
        {
            return m_scopes.top().getSymbols().end();
        }

        [[nodiscard]] inline std::vector<std::unique_ptr<const BoundDeclaration>>::const_iterator find(const std::string& name) const
        {
            for(auto it = m_scopes.top().getSymbols().begin(); it != m_scopes.top().getSymbols().end(); ++it)
                if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(it->get()))
                {
                    if(variable->getName() == name)
                        return it;
                }
                else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(it->get()))
                {
                    if(function->getName() == name)
                        return it;
                }
                else if(auto argument = dynamic_cast<const BoundArgumentDeclaration*>(it->get()))
                {
                    if(argument->getName() == name)
                        return it;
                }
                else 
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                        .message = "Unexpected declaration type during symbol binding process"});

                    return m_scopes.top().getSymbols().end();
                }
                
            return m_scopes.top().getSymbols().end();
        }

        [[nodiscard]] inline bool push(std::unique_ptr<const BoundDeclaration> symbol)
        {
            auto find = end();
            
            if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(symbol.get()))
                find = this->find(variable->getName());
            else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(symbol.get()))
                find = this->find(function->getName());
            else if(auto argument = dynamic_cast<const BoundArgumentDeclaration*>(symbol.get()))
                find = this->find(argument->getName());
            else 
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::ABT,
                    .message = "Unexpected declaration type during binding process"});
                return false;
            }          

            if(find == m_scopes.top().getSymbols().end())
            {
                m_scopes.top().getSymbols().push_back(std::move(symbol));
                return true;
            }
            else return false;
        }

        inline void beginScope()
        {
            if(m_scopes.empty())
                m_scopes.push(Scope{});
            else
                m_scopes.push(Scope(m_scopes.top()));
        }

        inline void endScope()
        {
            m_scopes.pop();
        }

        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundDeclaration>>& getSymbols() const
        {
            return m_scopes.top().getSymbols();
        } 
    private:
        std::stack<Scope> m_scopes;
    };

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

        inline void reset() { m_boundDeclarations.clear(); }
    private:
        static void reportInvalidBinaryOperator(BoundBinaryOperator::Kind operator_kind, Types::Type left_type, Types::Type right_type);
        static void reportInvalidUnaryOperator(BoundUnaryOperator::Kind operator_kind, Types::Type operand_type);
        [[nodiscard]] const std::unique_ptr<const BoundStatement> bindDeclarationStatement(const DeclarationStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundStatement> bindExpressionStatement(const ExpressionStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundStatement> bindScopeStatement(const ScopeStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundStatement> bindPutCharacterStatement(const PutCharacterStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundStatement> bindPutStringStatement(const PutStringStatement* statement);
        [[nodiscard]] const std::unique_ptr<const BoundDeclaration> bindVariableDeclaration(const VariableDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundDeclaration> bindArgumentDeclaration(const ArgumentDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundDeclaration> bindFunctionDeclaration(const FunctionDeclaration* declaration);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindIdentifierExpression(const IdentifierExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindIfElseExpression(const IfElseExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindWhileExpression(const WhileExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindVariableAssignmentExpression(const VariableAssignmentExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindFunctionCallExpression(const FunctionCallExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindLiteralExpression(const LiteralExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindUnaryExpression(const UnaryExpression* expression);
        [[nodiscard]] const std::unique_ptr<const BoundExpression> bindBinaryExpression(const BinaryExpression* expression);
        BoundUnaryOperator::Kind bindUnaryOperatorKind(Token::Type token_type);
        BoundBinaryOperator::Kind bindBinaryOperatorKind(Token::Type token_type);

        BoundDeclarations m_boundDeclarations;
    };
}