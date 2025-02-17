#pragma once
#include <linc/BoundTree.hpp>
#include <linc/Binder.hpp>
#include <linc/control_flow/ControlFlowGraph.hpp>
#define LINC_LOWERER_CONTROL_FLOW_IDENTIFIER_PREFIX '!'

namespace linc
{
    class Lowerer final
    {
    public:
        ControlFlowProgram lowerProgram(BoundProgram& program, const Binder& binder)
        {
            m_binder = &binder;
            m_identifierCounter = {};
            m_program = ControlFlowProgram{};
            m_program.functions.reserve(program.declarations.size());
            m_program.functions.push_back(ControlFlowGraph{}); // entry-point
            appendBlock(BasicBlock{});

            for(std::size_t i{0ul}; i < program.declarations.size(); ++i)
                if(auto generic = dynamic_cast<const BoundGenericDeclaration*>(program.declarations[i].get()))
                    for(const auto& instance: m_binder->getGenericInstanceMaps()[generic->getInstanceMapIndex()])
                    {
                        program.declarations.insert(program.declarations.begin() + i, instance.second->clone());
                        ++i;
                    }

            for(auto& declaration: program.declarations)
                if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(declaration.get()); variable && variable->getDefaultValue())
                {
                    m_program.globals.push_back(std::make_pair(mangleScope(variable->getName(), variable->getScopeIndex()), variable->getActualType()));
                    lowerExpression(variable->getDefaultValue());
                    appendAssignment(variable->getName(), variable->getActualType());
                }
            
            for(auto& declaration: program.declarations)
                if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(declaration.get()))
                    lowerFunction(function);
            
            auto main = std::make_unique<const BoundIdentifierExpression>(graph().prototype->getName(), graph().prototype->getFunctionType());
            auto call = std::make_unique<const BoundFunctionCallExpression>(graph().prototype->getReturnType(), std::move(main), std::vector<std::unique_ptr<const BoundExpression>>{});
            m_program.functions.push_back(std::move(m_program.functions[0ul]));
            graph().returnValue = std::move(call);
            appendExpression();
            m_program.functions[0ul] = std::move(graph());
            m_program.functions.pop_back();

            return std::move(m_program);
        }
    private:
        [[nodiscard]] inline ControlFlowGraph& graph() { return m_program.functions.back(); }
        [[nodiscard]] inline std::size_t blockIndex() const { return m_program.functions.back().blocks.size() - 1ul; }
        template <typename T>
        [[nodiscard]] inline T& as(std::size_t index) { return std::get<T>(graph().blocks[index]); }

        void appendStatement(const BoundStatement* statement)
        {
            struct Visitor
            {
                void operator()(ControlBlock& block)
                {
                    lowerer.appendBlockEdge(BasicBlock{});
                    lowerer.as<BasicBlock>(lowerer.blockIndex()).statements.push_back(statement->clone());
                }
                void operator()(BasicBlock& block) { block.statements.push_back(statement->clone()); }
                void operator()(UnreachableBlock& block) { } // todo: add warning
                void operator()(ConditionalBlock&) { throw LINC_EXCEPTION_ILLEGAL_STATE(lowerer.graph().blocks.back()); }
                void operator()(MapBlock&) { throw LINC_EXCEPTION_ILLEGAL_STATE(lowerer.graph().blocks.back()); }
                Lowerer& lowerer;
                const BoundStatement* statement;
            } visitor{*this, statement};

            std::visit(visitor, graph().blocks.back());
        }

        inline void appendExpression()
        {
            auto statement = std::make_unique<const BoundExpressionStatement>(std::move(graph().returnValue));
            appendStatement(statement.get());
        }

        inline void appendVoid()
        {
            graph().returnValue = std::make_unique<const BoundBlockExpression>(std::vector<std::unique_ptr<const BoundStatement>>{}, nullptr);
        }

        std::string appendVariable(Types::type type, const std::string& name, std::size_t scope_index)
        {
            type.isMutable = true;
            auto mangled_name = mangleScope(name, scope_index);
            auto variable = std::make_unique<const BoundVariableDeclaration>(type, mangled_name, nullptr, 0ul);
            auto statement = std::make_unique<const BoundDeclarationStatement>(std::move(variable));
            appendStatement(statement.get());
            return mangled_name;
        }

        inline std::string appendVariable(const Types::type& type)
        {
            return appendVariable(type, LINC_LOWERER_CONTROL_FLOW_IDENTIFIER_PREFIX + std::to_string(m_identifierCounter++), 0ul);
        }

        void appendAssignment(const std::string& name, const Types::type& type)
        {
            auto type_mutable = type;
            type_mutable.isMutable = true;
            auto assignment = std::make_unique<const BoundBinaryExpression>(
                std::make_unique<const BoundBinaryOperator>(BoundBinaryOperator::Kind::Assignment, type_mutable, type),
                std::make_unique<const BoundIdentifierExpression>(name, type),
                std::move(graph().returnValue)
            );
            auto statement = std::make_unique<const BoundExpressionStatement>(std::move(assignment));
            appendStatement(statement.get());
        }

        void setEdge(std::size_t index, std::size_t edge)
        {
            struct Visitor
            {
                void operator()(ControlBlock& block){ block.edge = edge; }
                void operator()(BasicBlock& block) { block.edge = edge; }
                void operator()(UnreachableBlock& block) {}
                void operator()(ConditionalBlock& block) { throw LINC_EXCEPTION_ILLEGAL_STATE(block); }
                void operator()(MapBlock& block) { throw LINC_EXCEPTION_ILLEGAL_STATE(block); }
                std::size_t edge;
            } visitor{edge};

            if(graph().blocks.size() > 1ul)
                std::visit(visitor, graph().blocks.at(index));
        }

        inline void setEdgeTrue(std::size_t index, std::size_t edge)
        {
            as<ConditionalBlock>(index).edgeTrue = edge;
        }

        inline void setEdgeFalse(std::size_t index, std::size_t edge)
        {
            as<ConditionalBlock>(index).edgeFalse = edge;
        }

        inline std::size_t appendBlock(Block&& block)
        {            
            graph().blocks.push_back(std::move(block));
            return blockIndex();
        }

        inline std::size_t appendBlockEdge(Block&& block)
        {            
            graph().blocks.push_back(std::move(block));
            setEdge(blockIndex() - 1ul, blockIndex());
            return blockIndex();
        }

        [[nodiscard]] static inline std::string mangleScope(const std::string& base, std::size_t scope_index)
        {
            return base + ':' + std::to_string(scope_index);
        }

        std::unique_ptr<const BoundFunctionPrototypeDeclaration> manglePrototype(const BoundFunctionPrototypeDeclaration* prototype) const
        {
            std::vector<std::unique_ptr<const BoundVariableDeclaration>> arguments;
            arguments.reserve(prototype->getArguments()->getList().size());
            for(const auto& element: prototype->getArguments()->getList())
                arguments.push_back(std::make_unique<const BoundVariableDeclaration>(element->getActualType(), 
                    mangleScope(element->getName(), element->getScopeIndex()), element->getDefaultValue()? element->getDefaultValue()->clone(): nullptr, 0ul));
            
            return std::make_unique<const BoundFunctionPrototypeDeclaration>(prototype->getFunctionType(), prototype->getName(), 
                std::make_unique<const BoundNodeListClause<BoundVariableDeclaration>>(std::move(arguments), prototype->getInfo()));
        }

        void lowerFunction(const BoundFunctionDeclaration* function)
        {
            m_program.functions.push_back(ControlFlowGraph{.prototype = manglePrototype(function->getPrototype())});
            appendBlock(BasicBlock{});
            lowerExpression(function->getBody());
            appendBlockEdge(ControlBlock{});
        }

        void lowerDeclaration(const BoundDeclaration* declaration)
        {
            if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(declaration))
                lowerVariableDeclaration(variable);

            else if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
                lowerFunctionDeclaration(function);

            else if(auto generic = dynamic_cast<const BoundGenericDeclaration*>(declaration))
                for(const auto& instance: m_binder->getGenericInstanceMaps()[generic->getInstanceMapIndex()])
                    lowerDeclaration(instance.second.get());

            else
            {
                auto statement = std::make_unique<const BoundDeclarationStatement>(declaration->clone());
                appendStatement(statement.get());
            }
        }

        void lowerFunctionDeclaration(const BoundFunctionDeclaration* declaration)
        {
            auto current_graph = std::move(m_program.functions.back());
            m_program.functions.pop_back();
            m_program.functions.push_back(ControlFlowGraph{.prototype = manglePrototype(declaration->getPrototype())});
            appendBlock(BasicBlock{});
            lowerExpression(declaration->getBody());
            appendBlockEdge(ControlBlock{});
            m_program.functions.push_back(std::move(current_graph));
        }

        void lowerVariableDeclaration(const BoundVariableDeclaration* declaration)
        {
            if(declaration->getDefaultValue())
            {
                lowerExpression(declaration->getDefaultValue());
                auto variable = std::make_unique<const BoundVariableDeclaration>(declaration->getActualType(), mangleScope(declaration->getName(), declaration->getScopeIndex()),
                    std::move(graph().returnValue), 0ul);
                auto statement = std::make_unique<const BoundDeclarationStatement>(std::move(variable));
                appendStatement(statement.get());
            }
        }

        void lowerStatement(const BoundStatement* statement)
        {
            if(auto return_statement = dynamic_cast<const BoundReturnStatement*>(statement))
            {
                lowerExpression(return_statement->getExpression());
                appendBlockEdge(UnreachableBlock{});
            }
            else if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
            {
                lowerExpression(expression_statement->getExpression());
                auto new_statement = std::make_unique<const BoundExpressionStatement>(std::move(graph().returnValue));
                appendStatement(new_statement.get());
            }
            else if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                lowerDeclaration(declaration_statement->getDeclaration());
        }

        void lowerExpression(const BoundExpression* expression)
        {
            if(auto literal_expression = dynamic_cast<const BoundLiteralExpression*>(expression))
                graph().returnValue = literal_expression->clone();
            
            else if(auto identifier_expression = dynamic_cast<const BoundIdentifierExpression*>(expression))
                graph().returnValue = std::make_unique<const BoundIdentifierExpression>(
                    mangleScope(identifier_expression->getValue(), identifier_expression->getScopeIndex()),
                    identifier_expression->getType());

            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
                graph().returnValue = type_expression->clone();

            else if(auto access_expression = dynamic_cast<const BoundAccessExpression*>(expression))
                return lowerAccessExpression(access_expression);

            else if(auto range_expression = dynamic_cast<const BoundRangeExpression*>(expression))
                return lowerRangeExpression(range_expression);

            else if(auto conversion_expression = dynamic_cast<const BoundConversionExpression*>(expression))
                return lowerConversionExpression(conversion_expression);

            else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
                return lowerBinaryExpression(binary_expression);

            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
                return lowerUnaryExpression(unary_expression);

            else if(auto block_expression = dynamic_cast<const BoundBlockExpression*>(expression))
                return lowerBlockExpression(block_expression);

            else if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
                return lowerIfExpression(if_expression);

            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
                return lowerWhileExpression(while_expression);

            else if(auto for_expression = dynamic_cast<const BoundForExpression*>(expression))
                return lowerForExpression(for_expression);

            else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
                return lowerFunctionCallExpression(function_call_expression);

            else if(auto external_call_expression = dynamic_cast<const BoundExternalCallExpression*>(expression))
                return lowerExternalCallExpression(external_call_expression);

            else if(auto match_expression = dynamic_cast<const BoundMatchExpression*>(expression))
                return lowerMatchExpression(match_expression);

            else throw LINC_EXCEPTION("LOWERER: EXPRESSION TYPE NOT IMPLEMENTED");
        }

        void lowerAccessExpression(const BoundAccessExpression* expression)
        {
            lowerExpression(expression->getBase());
            auto base = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundAccessExpression>(std::move(base), expression->getIndex(), expression->getType());
        }

        void lowerRangeExpression(const BoundRangeExpression* expression)
        {
            lowerExpression(expression->getBegin());
            auto begin = std::move(graph().returnValue);
            lowerExpression(expression->getEnd());
            auto end = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundRangeExpression>(std::move(begin), std::move(end), expression->getType());
        }

        void lowerConversionExpression(const BoundConversionExpression* expression)
        {
            lowerExpression(expression->getExpression());
            auto operand = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundConversionExpression>(std::move(operand), expression->getConversion()->clone());
        }

        void lowerBinaryExpression(const BoundBinaryExpression* expression)
        {
            lowerExpression(expression->getLeft());
            auto left = std::move(graph().returnValue);
            lowerExpression(expression->getRight());
            auto right = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundBinaryExpression>(expression->getOperator()->clone(), std::move(left), std::move(right));
        }

        void lowerUnaryExpression(const BoundUnaryExpression* expression)
        {
            lowerExpression(expression->getOperand());
            graph().returnValue = std::make_unique<const BoundUnaryExpression>(expression->getOperator()->clone(), std::move(graph().returnValue));
        }

        void lowerBlockExpression(const BoundBlockExpression* expression)
        {
            for(const auto& statement: expression->getStatements())
                lowerStatement(statement.get());
            if(expression->getTail())
                lowerExpression(expression->getTail());
            else appendVoid();
            
        }

        void lowerIfExpression(const BoundIfExpression* expression)
        {
            auto variable = appendVariable(expression->getType());
            lowerExpression(expression->getTestExpression());
            appendBlockEdge(ConditionalBlock{});
            auto conditional_index = blockIndex();
            as<ConditionalBlock>(conditional_index).condition = std::move(graph().returnValue);
            
            auto start_false_index = appendBlock(BasicBlock{});
            if(expression->getElseBody())
                lowerExpression(expression->getElseBody());
            else appendVoid();
            appendAssignment(variable, expression->getType());
            auto end_false_index = blockIndex();
            setEdgeFalse(conditional_index, start_false_index);

            auto start_true_index = appendBlock(BasicBlock{});
            lowerExpression(expression->getIfBody());
            appendAssignment(variable, expression->getType());
            auto end_true_index = blockIndex();
            setEdgeTrue(conditional_index, start_true_index);

            appendBlock(BasicBlock{});
            setEdge(end_false_index, blockIndex());
            setEdge(end_true_index, blockIndex());
            
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
        }

        void lowerWhileExpression(const BoundWhileExpression* expression)
        {
            auto else_body = expression->getElseBody();
            auto finally_body = expression->getFinallyBody();
            
            auto start_conditional_index = appendBlockEdge(BasicBlock{});
            auto variable = else_body? appendVariable(expression->getType()): std::string{};
            auto quit_variable = else_body || finally_body? appendVariable(Types::fromKind(Types::Kind::_bool)): std::string{};
            lowerExpression(expression->getTestExpression());
            auto end_conditional_index = appendBlockEdge(ConditionalBlock{});
            as<ConditionalBlock>(end_conditional_index).condition = std::move(graph().returnValue);
            
            auto start_true_index = appendBlock(BasicBlock{});
            lowerExpression(expression->getWhileBody());
            if(else_body)
                appendAssignment(variable, expression->getType());
            if(else_body || finally_body)
            {
                graph().returnValue = std::make_unique<const BoundLiteralExpression>(PrimitiveValue{true}, Types::fromKind(Types::Kind::_bool));
                appendAssignment(quit_variable, Types::fromKind(Types::Kind::_bool));
            }
            auto end_true_index = blockIndex();
            setEdgeTrue(end_conditional_index, start_true_index);
            setEdge(end_true_index, start_conditional_index);

            if(!else_body && !finally_body)
            {
                auto quit_index = appendBlock(BasicBlock{});
                setEdgeFalse(end_conditional_index, quit_index);
                return;
            }

            auto start_quit_conditional_index = appendBlock(BasicBlock{});
            setEdgeFalse(end_conditional_index, start_quit_conditional_index);
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(quit_variable, Types::fromKind(Types::Kind::_bool));
            auto end_quit_conditional_index = appendBlockEdge(ConditionalBlock{});
            as<ConditionalBlock>(end_quit_conditional_index).condition = std::move(graph().returnValue);

            auto start_else_index = appendBlock(BasicBlock{});
            if(else_body)
            {
                lowerExpression(else_body);
                appendAssignment(variable, expression->getType());
            }
            else appendVoid();
            auto end_else_index = blockIndex();
            setEdgeFalse(end_quit_conditional_index, start_else_index);

            auto start_finally_index = appendBlock(BasicBlock{});
            if(finally_body)
            {
                lowerExpression(finally_body);
                appendExpression();
            }
            else appendVoid();
            auto end_finally_index = blockIndex();
            setEdgeTrue(end_quit_conditional_index, start_finally_index);

            appendBlock(BasicBlock{});
            setEdge(end_else_index, blockIndex());
            setEdge(end_finally_index, blockIndex());
            
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
        }

        void lowerForExpression(const BoundForExpression* expression)
        {
            auto clause = expression->getForClause();
            if(auto legacy_for_clause = clause->getIfFirst())
            {
                lowerDeclaration(legacy_for_clause->getDeclaration());
                
                auto start_conditional_index = appendBlockEdge(BasicBlock{});
                auto variable = appendVariable(expression->getType());
                lowerExpression(legacy_for_clause->getTestExpression());
                auto end_conditional_index = appendBlockEdge(ConditionalBlock{});
                as<ConditionalBlock>(end_conditional_index).condition = std::move(graph().returnValue);

                auto start_body_index = appendBlock(BasicBlock{});
                lowerExpression(expression->getBody());
                appendAssignment(variable, expression->getType());
                lowerExpression(legacy_for_clause->getEndExpression());
                appendExpression();
                auto end_body_index = blockIndex();
                setEdgeTrue(end_conditional_index, start_body_index);

                appendBlock(BasicBlock{});
                setEdgeFalse(end_conditional_index, blockIndex());
                setEdge(end_body_index, start_conditional_index);
                
                graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
                return;
            }

            auto ranged_for_clause = clause->getSecond();
            auto identifier = ranged_for_clause->getIdentifier();
            auto iterable = ranged_for_clause->getExpression();

            if(iterable->getType().kind == Types::type::Kind::Structure)
            {
                graph().returnValue = std::make_unique<const BoundAccessExpression>(iterable->clone(), 0ul, identifier->getType());
                appendVariable(identifier->getType(), identifier->getValue(), identifier->getScopeIndex());
            }
            auto start_conditional_index = appendBlockEdge(BasicBlock{});
            auto variable = appendVariable(expression->getType());
            auto end = std::make_unique<const BoundAccessExpression>(iterable->clone(), 1ul, identifier->getType());
            lowerExpression(identifier);
            auto lowered_identifier = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundBinaryExpression>(std::make_unique<const BoundBinaryOperator>(BoundBinaryOperator::Kind::Less,
                identifier->getType(), identifier->getType()), std::move(lowered_identifier), std::move(end));
            auto end_conditional_index = appendBlockEdge(ConditionalBlock{});
            as<ConditionalBlock>(end_conditional_index).condition = std::move(graph().returnValue);

            auto start_body_index = appendBlock(BasicBlock{});
            lowerExpression(expression->getBody());
            appendAssignment(variable, expression->getType());
            graph().returnValue = std::make_unique<const BoundUnaryExpression>(
                std::make_unique<const BoundUnaryOperator>(BoundUnaryOperator::Kind::Increment, identifier->getType()), identifier->clone());
            appendExpression();
            auto end_body_index = blockIndex();
            setEdgeTrue(end_conditional_index, start_body_index);

            appendBlock(BasicBlock{});
            setEdgeFalse(end_conditional_index, blockIndex());
            setEdge(end_body_index, start_conditional_index);
            
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
        }

        void lowerFunctionCallExpression(const BoundFunctionCallExpression* expression)
        {
            std::vector<std::unique_ptr<const BoundExpression>> arguments;
            arguments.reserve(expression->getArguments().size());
            for(const auto& argument: expression->getArguments())
            {
                lowerExpression(argument.get());
                arguments.push_back(std::move(graph().returnValue));
            }
            lowerExpression(expression->getFunction());
            auto function = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundFunctionCallExpression>(expression->getType(), std::move(function), std::move(arguments));
        }

        void lowerExternalCallExpression(const BoundExternalCallExpression* expression)
        {
            std::vector<std::unique_ptr<const BoundExpression>> arguments;
            arguments.reserve(expression->getArguments().size());
            for(const auto& argument: expression->getArguments())
            {
                lowerExpression(argument.get());
                arguments.push_back(std::move(graph().returnValue));
            }
            auto function = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundExternalCallExpression>(expression->getType(), expression->getName(), std::move(arguments));
        }

        void lowerMatchExpression(const BoundMatchExpression* expression)
        {
            auto variable = appendVariable(expression->getType());
            lowerExpression(expression->getTestExpression());
            auto map_index = appendBlockEdge(MapBlock{});
            as<MapBlock>(map_index).testExpression = std::move(graph().returnValue);

            auto control_block = appendBlock(ControlBlock{});
            for(const auto& clause: expression->getClauses()->getList())
            {
                auto result_block = appendBlock(BasicBlock{});
                lowerExpression(clause->getExpression());
                appendAssignment(variable, expression->getType());
                setEdge(blockIndex(), control_block);

                for(const auto& value: clause->getValues()->getList())
                {
                    auto test_expression = appendBlock(BasicBlock{});
                    lowerExpression(value.get());
                    appendBlockEdge(UnreachableBlock{});
                    as<MapBlock>(map_index).cases.emplace_back(MapBlock::Case{std::move(graph().returnValue), test_expression, result_block});
                }
            }

            appendBlock(BasicBlock{});
            as<MapBlock>(map_index).edgeExit = blockIndex();
            as<ControlBlock>(control_block).edge = blockIndex(); 
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
        }
    private:
        mutable ControlFlowProgram m_program;
        mutable std::size_t m_identifierCounter;
        const Binder* m_binder{nullptr};
    };
}