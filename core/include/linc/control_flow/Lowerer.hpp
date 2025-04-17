#pragma once
#include <linc/BoundTree.hpp>
#include <linc/Binder.hpp>
#include <linc/control_flow/ControlFlowGraph.hpp>
#include <unistd.h>
#define LINC_LOWERER_CONTROL_FLOW_IDENTIFIER_PREFIX '!'

namespace linc
{
    struct LoopControlFlow final
    {
        std::string label;
        std::size_t continueEdge, breakEdge;
    };
    
    struct FunctionControlFlow final
    {
        std::string returnVariable;
        std::size_t returnEdge;
    };

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
                if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(declaration.get()))
                {
                    m_program.globals.push_back(std::make_pair(variable->getName(), variable->getActualType()));
                    if(variable->getDefaultValue())
                    {
                        lowerExpression(variable->getDefaultValue());
                        appendAssignment(variable->getName());
                    }
                }
            
            for(auto& declaration: program.declarations)
                if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(declaration.get()))
                    lowerFunction(function);
            
            // auto main = std::make_unique<const BoundIdentifierExpression>(graph().prototype->getName(), graph().prototype->getFunctionType());
            // auto call = std::make_unique<const BoundFunctionCallExpression>(graph().prototype->getReturnType(), std::move(main), std::vector<std::unique_ptr<const BoundExpression>>{});
            // m_program.functions.push_back(std::move(m_program.functions[0ul]));
            // graph().returnValue = std::move(call);
            // appendExpression();
            // m_program.functions[0ul] = std::move(graph());
            // m_program.functions.pop_back();
            appendVoid();
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
            graph().returnValue = std::make_unique<const BoundLiteralExpression>(PrimitiveValue::voidValue, Types::voidType);
        }

        void appendIdentifier(const std::string& name, const Types::type& type)
        {
            graph().returnValue = std::make_unique<const BoundIdentifierExpression>(name, type);
        }

        void appendLiteral(Types::Kind primitive, const PrimitiveValue& value)
        {
            graph().returnValue = std::make_unique<const BoundLiteralExpression>(value, Types::fromKind(primitive));
        }

        void appendDefaultLiteral(Types::Kind primitive)
        {
            graph().returnValue = std::make_unique<const BoundLiteralExpression>(PrimitiveValue::fromDefault(primitive), Types::fromKind(primitive));
        }

        std::string appendVariable(Types::type type, const std::string& name, std::size_t scope_index, std::unique_ptr<const BoundExpression> value = nullptr)
        {
            type.isMutable = true;
            auto mangled_name = mangleScope(name, scope_index);
            auto variable = std::make_unique<const BoundVariableDeclaration>(type, mangled_name, std::move(value), 0ul, std::nullopt);
            auto statement = std::make_unique<const BoundDeclarationStatement>(std::move(variable));
            appendStatement(statement.get());
            return mangled_name;
        }

        inline std::string appendVariable(const Types::type& type)
        {
            return appendVariable(type, LINC_LOWERER_CONTROL_FLOW_IDENTIFIER_PREFIX + std::to_string(m_identifierCounter++), 0ul);
        }

        void appendAssignment(const std::string& name)
        {
            const auto& type = graph().returnValue->getType();
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

        inline std::size_t reserveBlock()
        {
            graph().blocks.push_back(ControlBlock{});
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
                    mangleScope(element->getName(), element->getScopeIndex()), element->getDefaultValue()? element->getDefaultValue()->clone(): nullptr,
                    0ul, std::nullopt));
            
            return std::make_unique<const BoundFunctionPrototypeDeclaration>(prototype->getFunctionType(), prototype->getName(), 
                std::make_unique<const BoundNodeListClause<BoundVariableDeclaration>>(std::move(arguments), prototype->getInfo()), std::nullopt);
        }

        void lowerFunction(const BoundFunctionDeclaration* function)
        {
            m_program.functions.push_back(ControlFlowGraph{.prototype = manglePrototype(function->getPrototype())});
            
            auto return_block = reserveBlock();
            appendBlock(BasicBlock{});
            auto variable = appendVariable(function->getPrototype()->getReturnType());
            m_functions.push(FunctionControlFlow{.returnVariable = variable, .returnEdge = return_block});
            lowerExpression(function->getBody());
            appendAssignment(variable);
            appendIdentifier(variable, function->getPrototype()->getReturnType());
            m_functions.pop();
            appendBlockEdge(ControlBlock{});
            setEdge(return_block, blockIndex());
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
            lowerFunction(declaration);
            m_program.functions.push_back(std::move(current_graph));
        }

        void lowerVariableDeclaration(const BoundVariableDeclaration* declaration)
        {
            if(declaration->getDefaultValue())
                lowerExpression(declaration->getDefaultValue());

            appendVariable(declaration->getActualType(), declaration->getName(), declaration->getScopeIndex(), std::move(graph().returnValue));
        }

        void lowerStatement(const BoundStatement* statement)
        {
            if(auto return_statement = dynamic_cast<const BoundReturnStatement*>(statement))
            {
                lowerExpression(return_statement->getExpression());
                appendAssignment(m_functions.top().returnVariable);
                appendBlockEdge(ControlBlock{});
                setEdge(blockIndex(), m_functions.top().returnEdge);
                appendBlock(BasicBlock{});
            }
            else if(auto break_statement = dynamic_cast<const BoundBreakStatement*>(statement))
            {
                appendBlockEdge(ControlBlock{});
                for(std::size_t i = m_loops.size(); i != 0ul; --i)
                    if(m_loops[i - 1ul].label == break_statement->getLabel())
                    {
                        setEdge(blockIndex(), m_loops[i - 1ul].breakEdge);
                        break;
                    }
                appendBlock(BasicBlock{});
            }
            else if(auto continue_statement = dynamic_cast<const BoundContinueStatement*>(statement))
            {
                appendBlockEdge(ControlBlock{});
                for(std::size_t i = m_loops.size(); i != 0ul; --i)
                    if(m_loops[i - 1ul].label == continue_statement->getLabel())
                    {
                        setEdge(blockIndex(), m_loops[i - 1ul].continueEdge);
                        break;
                    }
                appendBlock(BasicBlock{});
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

            else if(auto array_initializer_expression = dynamic_cast<const BoundArrayInitializerExpression*>(expression))
                return lowerArrayInitializerExpression(array_initializer_expression);

            else if(auto structure_initializer_expression = dynamic_cast<const BoundStructureInitializerExpression*>(expression))
                return lowerStructureInitializerExpression(structure_initializer_expression);

            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
                graph().returnValue = type_expression->clone();

            else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
                return lowerIndexExpression(index_expression);

            else if(auto access_expression = dynamic_cast<const BoundAccessExpression*>(expression))
                return lowerAccessExpression(access_expression);

            else if(auto enumerator_expression = dynamic_cast<const BoundEnumeratorExpression*>(expression))
                return lowerEnumeratorExpression(enumerator_expression);

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

            else throw LINC_EXCEPTION_ILLEGAL_VALUE(expression);
        }

        void lowerStructureInitializerExpression(const BoundStructureInitializerExpression* expression)
        {
            std::vector<std::unique_ptr<const BoundExpression>> fields;
            fields.reserve(expression->getFields().size());
            for(const auto& field: expression->getFields())
            {
                lowerExpression(field.get());
                fields.push_back(std::move(graph().returnValue));
            }
            graph().returnValue = std::make_unique<const BoundStructureInitializerExpression>(expression->getName(), std::move(fields), expression->getType());
        }

        void lowerArrayInitializerExpression(const BoundArrayInitializerExpression* expression)
        {
            std::vector<std::unique_ptr<const BoundExpression>> values;
            values.reserve(expression->getValues().size());
            for(const auto& value: expression->getValues())
            {
                lowerExpression(value.get());
                values.push_back(std::move(graph().returnValue));
            }
            graph().returnValue = std::make_unique<const BoundArrayInitializerExpression>(std::move(values), expression->getType());
        }

        void lowerIndexExpression(const BoundIndexExpression* expression)
        {
            lowerExpression(expression->getArray());
            auto array = std::move(graph().returnValue);
            lowerExpression(expression->getIndex());
            graph().returnValue = std::make_unique<const BoundIndexExpression>(std::move(array), std::move(graph().returnValue), expression->getType());
        }

        void lowerAccessExpression(const BoundAccessExpression* expression)
        {
            lowerExpression(expression->getBase());
            auto base = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundAccessExpression>(std::move(base), expression->getIndex(), expression->getType());
        }

        void lowerEnumeratorExpression(const BoundEnumeratorExpression* expression)
        {
            if(expression->getValue())
                lowerExpression(expression->getValue());
            else appendVoid();
            graph().returnValue = std::make_unique<const BoundEnumeratorExpression>(expression->getEnumerationName(), expression->getEnumeratorIndex(),
                std::move(graph().returnValue), expression->getType());
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
            appendAssignment(variable);
            auto end_false_index = blockIndex();
            setEdgeFalse(conditional_index, start_false_index);

            auto start_true_index = appendBlock(BasicBlock{});
            lowerExpression(expression->getIfBody());
            appendAssignment(variable);
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
            
            auto exit_control_index = reserveBlock();
            m_loops.push_back(LoopControlFlow{.label = expression->getLabel(), .continueEdge = start_conditional_index, .breakEdge = exit_control_index});
            auto start_true_index = appendBlock(BasicBlock{});
            lowerExpression(expression->getWhileBody());
            m_loops.pop_back();
            if(else_body)
                appendAssignment(variable);
            if(else_body || finally_body)
            {
                appendLiteral(Types::Kind::_bool, PrimitiveValue(true));
                appendAssignment(quit_variable);
            }
            auto end_true_index = blockIndex();
            setEdgeTrue(end_conditional_index, start_true_index);
            setEdge(end_true_index, start_conditional_index);

            if(!else_body && !finally_body)
            {
                appendBlock(BasicBlock{});
                setEdgeFalse(end_conditional_index, blockIndex());
                setEdge(exit_control_index, blockIndex());
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
                appendAssignment(variable);
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
            setEdge(exit_control_index, blockIndex());            
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

                auto exit_control_index = reserveBlock();
                auto end_expression_control_index = reserveBlock();
                auto start_body_index = appendBlock(BasicBlock{});
                m_loops.push_back(LoopControlFlow{.label = expression->getLabel(), .continueEdge = end_expression_control_index, .breakEdge = exit_control_index});
                lowerExpression(expression->getBody());
                m_loops.pop_back();
                appendBlockEdge(BasicBlock{});
                setEdge(end_expression_control_index, blockIndex());
                appendAssignment(variable);
                lowerExpression(legacy_for_clause->getEndExpression());
                appendExpression();
                auto end_body_index = blockIndex();
                setEdgeTrue(end_conditional_index, start_body_index);

                appendBlock(BasicBlock{});
                setEdge(exit_control_index, blockIndex());
                setEdgeFalse(end_conditional_index, blockIndex());
                setEdge(end_body_index, start_conditional_index);
                
                graph().returnValue = std::make_unique<const BoundIdentifierExpression>(variable, expression->getType());
                return;
            }

            auto ranged_for_clause = clause->getSecond();
            auto identifier = ranged_for_clause->getIdentifier();
            lowerExpression(ranged_for_clause->getExpression());
            auto iterable = std::move(graph().returnValue);
            std::string secondary_variable{};
            auto secondary_type = Types::type(Types::Kind::u64, true);
            auto base_type = iterable->getType().kind == Types::type::Kind::Array? *iterable->getType().array.baseType:
                Types::fromKind(Types::Kind::_char);

            switch(iterable->getType().kind)
            {
            case Types::type::Kind::Structure:
                graph().returnValue = std::make_unique<const BoundAccessExpression>(iterable->clone(), 0ul, identifier->getType());
                appendVariable(identifier->getType(), identifier->getValue(), identifier->getScopeIndex(), std::move(graph().returnValue));
                break;
            case Types::type::Kind::Primitive:
                if(iterable->getType().primitive != Types::Kind::string)
                    throw LINC_EXCEPTION_ILLEGAL_VALUE(iterable);
                [[fallthrough]];
            case Types::type::Kind::Array:
            {
                appendDefaultLiteral(Types::Kind::u64);
                auto index = graph().returnValue->clone();
                secondary_variable = appendVariable(secondary_type);

                graph().returnValue = std::make_unique<const BoundIndexExpression>(iterable->clone(), std::move(index), base_type);
                appendVariable(identifier->getType(), identifier->getValue(), identifier->getScopeIndex());
                break;
            } 
            default: throw LINC_EXCEPTION_ILLEGAL_STATE(iterable->getType().kind);
            }

            auto start_conditional_index = appendBlockEdge(BasicBlock{});
            auto variable = appendVariable(expression->getType());
            std::unique_ptr<const BoundExpression> end;
            switch(iterable->getType().kind)
            {
            case Types::type::Kind::Structure:
                end = std::make_unique<const BoundAccessExpression>(iterable->clone(), 1ul, identifier->getType());
                lowerExpression(identifier);
                break;
            case Types::type::Kind::Array:
            case Types::type::Kind::Primitive:
                end = std::make_unique<const BoundUnaryExpression>(
                    std::make_unique<const BoundUnaryOperator>(BoundUnaryOperator::Kind::UnaryPlus, iterable->getType()),
                    iterable->clone());
                graph().returnValue = std::make_unique<const BoundIdentifierExpression>(secondary_variable, secondary_type);
                break;
            default: throw LINC_EXCEPTION_ILLEGAL_STATE(iterable->getType().kind);
            }

            auto lowered_identifier = std::move(graph().returnValue);
            graph().returnValue = std::make_unique<const BoundBinaryExpression>(std::make_unique<const BoundBinaryOperator>(BoundBinaryOperator::Kind::Less,
                identifier->getType(), identifier->getType()), std::move(lowered_identifier), std::move(end));
            auto end_conditional_index = appendBlockEdge(ConditionalBlock{});
            as<ConditionalBlock>(end_conditional_index).condition = std::move(graph().returnValue);

            auto exit_control_index = reserveBlock();
            auto increment_control_index = reserveBlock();
            auto start_body_index = appendBlock(BasicBlock{});
            if(iterable->getType().kind != Types::type::Kind::Structure)
            {
                graph().returnValue = std::make_unique<const BoundIndexExpression>(iterable->clone(),
                    std::make_unique<const BoundIdentifierExpression>(secondary_variable, secondary_type),
                    base_type);
                appendAssignment(mangleScope(identifier->getValue(), identifier->getScopeIndex()));
            }

            m_loops.push_back(LoopControlFlow{.label = expression->getLabel(), .continueEdge = increment_control_index, .breakEdge = exit_control_index});
            lowerExpression(expression->getBody());
            m_loops.pop_back();
            appendBlockEdge(BasicBlock{});
            setEdge(increment_control_index, blockIndex());
            appendAssignment(variable);
            if(iterable->getType().kind == Types::type::Kind::Structure)
            {
                std::unique_ptr<const BoundExpression> increment = std::make_unique<const BoundUnaryExpression>(
                    std::make_unique<const BoundUnaryOperator>(BoundUnaryOperator::Kind::Increment, identifier->getType()), identifier->clone());
                lowerExpression(increment.get());
            }
            else
                graph().returnValue = std::make_unique<const BoundUnaryExpression>(
                    std::make_unique<const BoundUnaryOperator>(BoundUnaryOperator::Kind::Increment, secondary_type),
                        std::make_unique<const BoundIdentifierExpression>(secondary_variable, secondary_type, 0ul));

            appendExpression();
            auto end_body_index = blockIndex();
            setEdgeTrue(end_conditional_index, start_body_index);

            appendBlock(BasicBlock{});
            setEdge(exit_control_index, blockIndex());
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
                appendAssignment(variable);
                setEdge(blockIndex(), control_block);

                for(const auto& value: clause->getValues()->getList())
                {
                    auto test_block = appendBlock(BasicBlock{});
                    [&]() {
                        auto identifier = dynamic_cast<const BoundIdentifierExpression*>(value.get());
                        if(identifier && value->getType() == Types::voidType)
                        {
                            appendVariable(identifier->getType(), identifier->getValue(), identifier->getScopeIndex());
                            lowerExpression(expression->getTestExpression());
                            return;
                        }
                        lowerExpression(value.get());
                    }();
                    appendBlockEdge(UnreachableBlock{});
                    as<MapBlock>(map_index).cases.emplace_back(MapBlock::Case{std::move(graph().returnValue), test_block, result_block});
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
        mutable std::stack<FunctionControlFlow> m_functions;
        mutable std::vector<LoopControlFlow> m_loops;
        const Binder* m_binder{nullptr};
    };
}