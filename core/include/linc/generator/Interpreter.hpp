#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>
#include <linc/Include.hpp>
#include <linc/Binder.hpp>
#include <linc/generator/ControlFlowExceptions.hpp>
#ifdef LINC_LINUX
#include <unistd.h>
#endif

#define LINC_EXIT_PROGRAM_FAILURE 5
#define LINC_EXIT_PROGRAM_SUCCESS 0

namespace linc
{
    class Interpreter final
    {
    public:
        [[nodiscard("The return value of this function must match that of the environment's entry point (i.e. main()).")]]
        int evaluateProgram(const BoundProgram* program, Binder& binder, std::unique_ptr<const ArrayInitializerExpression> argument_list)
        {
            for(const auto& declaration: program->declarations)
                evaluateDeclaration(declaration.get());
        
            auto main_name = linc::PrimitiveValue(std::string{"main"});
            auto find_main = binder.find(main_name.getString());
            if(!find_main)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("Call to undeclared entry-point function $.", main_name)
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }
            else if(auto main = dynamic_cast<const BoundFunctionDeclaration*>(find_main.get()); !main)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("The symbol-name $ is reserved for the entry point function, and cannot be used for other symbol types.",
                        main_name)
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }

            auto main = static_cast<const BoundFunctionDeclaration*>(find_main.get());
            auto main_argument_list = std::vector<NodeListClause<Expression>::DelimitedNode>{};

            if(!main->getArguments().empty())
                main_argument_list.push_back(NodeListClause<Expression>::DelimitedNode{
                    .delimiter = std::nullopt,
                    .node = std::move(argument_list)
                });

            auto main_call = std::make_unique<const linc::CallExpression>(
                linc::Token{.type = linc::Token::Type::ParenthesisLeft},
                linc::Token{.type = linc::Token::Type::ParenthesisRight},
                std::make_unique<const IdentifierExpression>(linc::Token{.type = linc::Token::Type::Identifier, .value = main->getName()}),
                std::make_unique<const NodeListClause<Expression>>(std::move(main_argument_list), Token::Info{}), false);

            auto bound_main_call = binder.bindExpression(main_call.get());

            bool errors{false};
            for(const auto& report: Reporting::getReports())
                if(report.type == Reporting::Type::Error)
                    errors = true;

            if(errors)
                return LINC_EXIT_PROGRAM_FAILURE;

            switch(bound_main_call->getType().primitive)
            {
            case Types::Kind::u8: return evaluateExpression(bound_main_call.get()).getPrimitive().getU8();
            case Types::Kind::i8: return evaluateExpression(bound_main_call.get()).getPrimitive().getI8();
            case Types::Kind::i16: return evaluateExpression(bound_main_call.get()).getPrimitive().getI16();
            case Types::Kind::i32: return evaluateExpression(bound_main_call.get()).getPrimitive().getI32();
            case Types::Kind::_void:
                evaluateExpression(bound_main_call.get());
                return LINC_EXIT_PROGRAM_SUCCESS;
            default:
                Reporting::push({Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("Defined function main() using return type $. "
                        "Main only supports void and signed integral return types of at most 32 bits, "
                        "as well as 8-bit unsigned integers.",
                        bound_main_call->getType())
                }});
                return LINC_EXIT_PROGRAM_FAILURE;
            }
        }

        Value evaluateNode(const BoundNode* node)
        {
            if(auto expression = dynamic_cast<const BoundExpression*>(node))
                return evaluateExpression(std::move(expression));

            else if(auto statement = dynamic_cast<const BoundStatement*>(node))
                return evaluateStatement(std::move(statement));
            
            else if(auto declaration = dynamic_cast<const BoundDeclaration*>(node))
                return evaluateDeclaration(std::move(declaration));

            throw LINC_EXCEPTION_INVALID_INPUT("Encountered unreognized node during evaluation");
        }

        Value evaluateStatement(const BoundStatement* statement)
        {
            if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                return evaluateDeclaration(declaration_statement->getDeclaration());
            
            else if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
                return (evaluateExpression(expression_statement->getExpression()), PrimitiveValue::voidValue);

            else if(auto return_statement = dynamic_cast<const BoundReturnStatement*>(statement))
                throw ReturnException{return_statement->getExpression()? evaluateExpression(return_statement->getExpression()): PrimitiveValue::voidValue};

            else if(auto break_statement = dynamic_cast<const BoundBreakStatement*>(statement))
                throw BreakException{break_statement->getLabel()};

            else if(auto continue_statement = dynamic_cast<const BoundContinueStatement*>(statement))
                throw ContinueException{continue_statement->getLabel()};

            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized statement type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        Value evaluateDeclaration(const BoundDeclaration* declaration)
        {
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            {
                auto value = variable_declaration->getDefaultValue()? evaluateExpression(variable_declaration->getDefaultValue()):
                    Value::fromDefault(variable_declaration->getActualType());
                if(auto array = value.getIfArray(); array && array->getCount() == 0ul)
                    value = ArrayValue::fromDefault(*variable_declaration->getActualType().array.baseType, 0ul);
                m_identifiers.append(variable_declaration->getName(), value);

                return PrimitiveValue::voidValue;
            }
            else if(auto function_declaration = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
            {
                std::vector<std::string> argument_names;
                argument_names.reserve(function_declaration->getArguments().size());
                for(const auto& argument: function_declaration->getArguments())
                    argument_names.push_back(argument->getName());
                FunctionValue value(function_declaration->getName(), std::move(argument_names), function_declaration->getBody()->clone());
                m_identifiers.append(function_declaration->getName(), value);
                return PrimitiveValue::voidValue;
            }
            else if(dynamic_cast<const BoundExternalDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
            else if(dynamic_cast<const BoundStructureDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
            else if(auto enumeration_declaration = dynamic_cast<const BoundEnumerationDeclaration*>(declaration))
            {
                m_enumerations.append(enumeration_declaration->getName(), enumeration_declaration->getActualType().enumeration);
                return PrimitiveValue::voidValue;
            }
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized declaration type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        Value evaluateExpression(const BoundExpression* expression)
        {
            if(auto literal_expression = dynamic_cast<const BoundLiteralExpression*>(expression))
            {
                return literal_expression->getValue();
            }
            else if(auto block_expression = dynamic_cast<const BoundBlockExpression*>(expression))
            {
                Value value = PrimitiveValue::voidValue; 
                beginScope();
                for(std::size_t i{0ul}; i < block_expression->getStatements().size(); ++i)
                {
                    const auto& statement = block_expression->getStatements()[i];
                    evaluateStatement(statement.get());
                }
                
                value = block_expression->getTail()? evaluateExpression(block_expression->getTail()): value;
                endScope();
                return value;
            }
            else if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
            {
                auto test = evaluateExpression(if_expression->getTestExpression()).getPrimitive().getBool();

                if(test)
                    return evaluateExpression(if_expression->getIfBody());
                else if(if_expression->hasElse())
                    return evaluateExpression(if_expression->getElseBody());
                else return PrimitiveValue::voidValue;
            }
            else if(auto for_expression = dynamic_cast<const BoundForExpression*>(expression))
            {
                beginScope();
                const auto& clause = for_expression->getForClause();

                auto loop = [this, &for_expression](Value& return_value, std::function<bool()> condition, std::function<void()> end){
                    for(;condition(); end())
                    {
                        try
                        {
                            return_value = evaluateExpression(for_expression->getBody());
                        }
                        catch(const BreakException& break_exception)
                        {
                            if(for_expression->getLabel() == break_exception.label || break_exception.label.empty())
                                break;
                            else throw (endScope(), break_exception);
                        }
                        catch(const ContinueException& continue_exception)
                        {
                            if(for_expression->getLabel() == continue_exception.label || continue_exception.label.empty())
                                continue;
                            else throw (endScope(), continue_exception);
                        }
                    }
                };

                if(auto legacy_clause = clause->getIfFirst())
                {
                    evaluateDeclaration(legacy_clause->getDeclaration());
                    Value return_value = Value::fromDefault(for_expression->getBody()->getType());

                    loop(return_value, [&](){ return evaluateExpression(legacy_clause->getTestExpression()).getPrimitive().getBool(); },
                        [&](){ evaluateExpression(legacy_clause->getEndExpression()); });

                    endScope();
                    return return_value;
                }

                auto ranged_clause = clause->getSecond();
                auto name = ranged_clause->getIdentifier()->getValue();
                auto expression_type = ranged_clause->getExpression()->getType();
                auto iterable = evaluateExpression(ranged_clause->getExpression());
                if(expression_type.kind == Types::type::Kind::Structure)
                {
                    auto begin = iterable.getStructure().at(0ul);
                    auto end = iterable.getStructure().at(1ul);
                    auto reverse = iterable.getStructure().at(2ul).getPrimitive().getBool();
                    Value return_value = Value::fromDefault(for_expression->getBody()->getType());

                    if(reverse)
                    {
                        --end;
                        m_identifiers.append(name, end);
                        loop(return_value, [&](){ return *m_identifiers.get(name) != begin; }, [&](){ --(*m_identifiers.get(name)); });
                        if(*m_identifiers.get(name) == begin)
                        {
                            try
                            {
                                return_value = evaluateExpression(for_expression->getBody());
                            }
                            catch(const BreakException& break_exception)
                            {
                                if(for_expression->getLabel() == break_exception.label || break_exception.label.empty());
                                else throw (endScope(), break_exception);
                            }
                            catch(const ContinueException& continue_exception)
                            {
                                if(for_expression->getLabel() == continue_exception.label || continue_exception.label.empty());
                                else throw (endScope(), continue_exception);
                            }
                        }
                    }
                    else
                    {
                        m_identifiers.append(name, begin);
                        loop(return_value, [&](){ return *m_identifiers.get(name) != end; }, [&](){ ++(*m_identifiers.get(name)); });
                    }
                    
                    endScope();
                    return return_value;
                }
                
                if(expression_type.kind == Types::type::Kind::Primitive && expression_type.primitive == Types::type::Primitive::string)
                {
                    auto string = iterable.getPrimitive().getString();
                    auto c_string = string.c_str();
                    
                    Value return_value = Value::fromDefault(Types::fromKind(Types::Kind::_char));
                    m_identifiers.append(name, PrimitiveValue(c_string[0ul]));
                    loop(return_value, [&](){ return *c_string; }, [&](){ ++c_string; *m_identifiers.get(name) = PrimitiveValue(*c_string); });
                    
                    endScope();
                    return return_value;
                }

                auto array = iterable.getArray();
                Value return_value = Value::fromDefault(*expression_type.array.baseType);
                std::size_t i{0ul};
                m_identifiers.append(name, array.getCount() == 0ul? PrimitiveValue::voidValue: array.get(0ul));
                loop(return_value, [&](){ auto test = i < array.getCount(); if(test) *m_identifiers.get(name) = array.get(i); return test; }, [&](){ ++i; });
                
                endScope();
                return return_value;
            }
            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
            {
                Value return_value = PrimitiveValue::voidValue;
                bool evaluated{false};

                while(evaluateExpression(while_expression->getTestExpression()).getPrimitive().getBool())
                {
                    evaluated = true;
                    try
                    {
                        return_value = evaluateExpression(while_expression->getWhileBody());
                    }
                    catch(const BreakException& break_exception)
                    {
                        if(while_expression->getLabel() == break_exception.label || break_exception.label.empty())
                            break;
                        else throw break_exception;
                    }
                    catch(const ContinueException& continue_exception)
                    {
                        if(while_expression->getLabel() == continue_exception.label || continue_exception.label.empty())
                            continue;
                        else throw continue_exception;
                    }
                }
                
                auto finally = while_expression->getFinallyBody();
                auto _else = while_expression->getElseBody();

                if(evaluated && finally)
                    return_value = evaluateExpression(finally);
                else if(!evaluated && _else)
                    return_value = evaluateExpression(_else);

                return return_value;
            }
            else if(auto match_expression = dynamic_cast<const BoundMatchExpression*>(expression))
            {
                auto test_expression = evaluateExpression(match_expression->getTestExpression());
                for(const auto& clause: match_expression->getClauses()->getList())
                {
                    for(const auto& value: clause->getValues()->getList())
                    {
                        beginScope();
                        [&, this]()
                        {
                            if(!test_expression.getIfEnumerator()) return;
                            auto enumerator = dynamic_cast<const BoundEnumeratorExpression*>(value.get());
                            if(!enumerator || match_expression->getTestExpression()->getType().kind != Types::type::Kind::Enumeration) return;
                            auto identifier = dynamic_cast<const BoundIdentifierExpression*>(enumerator->getValue());
                            if(!identifier || m_identifiers.find(identifier->getValue())) return;
                            m_identifiers.append(identifier->getValue(), test_expression.getEnumerator().getValue());    
                        }();
                        if(evaluateExpression(value.get()) == test_expression)
                        {
                            auto result = evaluateExpression(clause->getExpression());
                            endScope();
                            return result;
                        }
                        else endScope();
                    }
                }
                return Value::fromDefault(match_expression->getType());
            }
            else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
            {
                Value result = Value::fromDefault(binary_expression->getType());
                
                if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalAnd)
                {
                    if(evaluateExpression(binary_expression->getLeft()).getPrimitive().getBool())
                        return PrimitiveValue(evaluateExpression(binary_expression->getRight()).getPrimitive().getBool());
                    else return PrimitiveValue(false);
                }
                else if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalOr)
                {
                    if(evaluateExpression(binary_expression->getLeft()).getPrimitive().getBool())
                        return PrimitiveValue(true);
                    else return PrimitiveValue(evaluateExpression(binary_expression->getRight()).getPrimitive().getBool());
                }
                else if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::Addition)
                {
                    auto left = evaluateExpression(binary_expression->getLeft());
                    auto right = evaluateExpression(binary_expression->getRight());

                    result = left + right;
                }

                auto left = evaluateExpression(binary_expression->getLeft());
                auto right = evaluateExpression(binary_expression->getRight());

                switch(binary_expression->getOperator()->getKind())
                {
                case BoundBinaryOperator::Kind::Assignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), right);
                case BoundBinaryOperator::Kind::AdditionAssignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left + right);
                case BoundBinaryOperator::Kind::SubtractionAssignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() - right.getPrimitive());
                case BoundBinaryOperator::Kind::MultiplicationAssignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() * right.getPrimitive());
                case BoundBinaryOperator::Kind::DivisionAssignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() / right.getPrimitive());
                case BoundBinaryOperator::Kind::ModuloAssignment:
                    return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() % right.getPrimitive());
                case BoundBinaryOperator::Kind::Subtraction:
                    result = left - right;
                    break;
                case BoundBinaryOperator::Kind::Multiplication:
                    result = left * right;
                    break;
                case BoundBinaryOperator::Kind::Division:
                    if(right.getPrimitive().isZero())
                        return (Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = Logger::format("Attempted division by zero. Operands are `$` and `$` (`$`).",
                                left, right, binary_expression->getLeft()->getType())
                        }), PrimitiveValue::invalidValue);
                    result = left / right;
                    break;
                case BoundBinaryOperator::Kind::Modulo:
                    if(right.getPrimitive().isZero())
                        return (Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = Logger::format("Attempted modulo division by zero. Operands are `$` and `$` (`$`).",
                                left, right, binary_expression->getLeft()->getType())
                        }), PrimitiveValue::invalidValue);
                    result = left % right;
                    break;
                case BoundBinaryOperator::Kind::Equals:
                    result = PrimitiveValue(left == right);
                    break;
                case BoundBinaryOperator::Kind::NotEquals:
                    result = PrimitiveValue(left != right);
                    break;
                case BoundBinaryOperator::Kind::Greater:
                    result = PrimitiveValue(left > right);
                    break;
                case BoundBinaryOperator::Kind::Less:
                    result = PrimitiveValue(left < right);
                    break;
                case BoundBinaryOperator::Kind::GreaterEqual:
                    result = PrimitiveValue(left >= right);
                    break;
                case BoundBinaryOperator::Kind::LessEqual:
                    result = PrimitiveValue(left <= right);
                    break;
                case BoundBinaryOperator::Kind::BitwiseAnd:
                    result = left & right;
                    break;
                case BoundBinaryOperator::Kind::BitwiseOr:
                    result = left | right;
                    break;
                case BoundBinaryOperator::Kind::BitwiseXor:
                    result = left ^ right;
                    break;
                case BoundBinaryOperator::Kind::BitwiseShiftLeft:
                    result = left << right;
                    break;
                case BoundBinaryOperator::Kind::BitwiseShiftRight:
                    result = left >> right;
                    break;
                default: break;
                }

                if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                    || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                    return result.getPrimitive().convert(binary_expression->getType().primitive);
                else return result;
            }
            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
            {
                Value result = Value::fromDefault(unary_expression->getType());
                
                if(unary_expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Typeof)
                    return PrimitiveValue(unary_expression->getOperand()->getType());

                auto operand = evaluateExpression(unary_expression->getOperand());

                switch(unary_expression->getOperator()->getKind())
                {
                case BoundUnaryOperator::Kind::Increment:
                    result = evaluateMutableOperator(unary_expression->getType(), unary_expression->getOperand(),
                        operand + PrimitiveValue{1}.convert(unary_expression->getType().primitive));
                    break;
                case BoundUnaryOperator::Kind::Decrement:
                    result = evaluateMutableOperator(unary_expression->getType(), unary_expression->getOperand(), 
                        operand - PrimitiveValue{1}.convert(unary_expression->getType().primitive));
                    break;
                case BoundUnaryOperator::Kind::Stringify:
                    result = PrimitiveValue(operand.toApplicationString());
                    break;
                case BoundUnaryOperator::Kind::UnaryPlus:
                    if(operand.getIfArray())
                        result = PrimitiveValue(operand.getArray().getCount());
                    else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::String)
                        result = PrimitiveValue(static_cast<Types::u64>(operand.getPrimitive().getString().size()));
                    else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::Character)
                        result = PrimitiveValue(+operand.getPrimitive().getChar());
                    else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::Boolean)
                        result = PrimitiveValue(static_cast<Types::i32>(operand.getPrimitive().getBool()));
                    else result = operand;
                    break;
                case BoundUnaryOperator::Kind::UnaryMinus:
                    if(auto structure = operand.getIfStructure())
                    {
                        structure->at(2ul) = PrimitiveValue{!structure->at(2ul).getPrimitive().getBool()};
                        result = Value{*structure};
                        break;
                    }
                    else result = -operand;
                    break;
                case BoundUnaryOperator::Kind::LogicalNot:
                    result = PrimitiveValue(!operand.getPrimitive().getBool());
                    break;
                case BoundUnaryOperator::Kind::BitwiseNot:
                    result = ~operand;
                    break;
                default: 
                    result = PrimitiveValue::invalidValue;
                }

                if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                    || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                    return result.getPrimitive().convert(unary_expression->getType().primitive);
                else return result;
            }
            else if(auto identifier_expression = dynamic_cast<const BoundIdentifierExpression*>(expression))
            {
                auto find = m_identifiers.find(identifier_expression->getValue());
                
                if(!find)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Identifier `$` does not exist!", identifier_expression->getValue())
                    }), PrimitiveValue::invalidValue);

                return *find;
            }
            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
            {
                return PrimitiveValue(type_expression->getActualType());
            }
            else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
            {
                beginScope();
                auto function = evaluateExpression(function_call_expression->getFunction());
                for(std::size_t i{0ul}; i < function_call_expression->getArguments().size(); ++i)
                {
                    auto value = evaluateExpression(function_call_expression->getArguments()[i].get());
                    m_identifiers.append(function.getFunction().getArgumentNames().at(i), value);
                }

                try
                {
                    auto result = evaluateExpression(function.getFunction().getBody());
                    endScope();
                    return result;
                }
                catch(const ReturnException& return_exception)
                {
                    endScope();
                    return return_exception.returnValue;
                }
            }
            else if(auto external_call = dynamic_cast<const BoundExternalCallExpression*>(expression))
            {
                const auto& name = external_call->getName();
                
                if(name == "puts")
                {
                    fputs(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString().c_str(), stdout);
                    return PrimitiveValue::voidValue;
                }
                else if(name == "putln")
                {
                    fputs((evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString() + '\n').c_str(), stdout);
                    return PrimitiveValue::voidValue;
                }
                else if(name == "putc")
                {
                    fputc(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getChar(), stdout);
                    return PrimitiveValue::voidValue;
                }
                else if(name == "readc")
                {
                    return PrimitiveValue(static_cast<char>(std::getchar()));
                }
                else if(name == "readraw")
                {
                    std::string result;
                    std::getline(std::cin, result);
                    return linc::PrimitiveValue(result);
                }
                else if(name == "readln")
                {
                    auto prompt = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString();
                    return linc::PrimitiveValue(Logger::read(prompt));
                }
                else if(name == "sys_write")
                {
                #ifdef LINC_LINUX                        
                    auto file_descriptor = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive();
                    auto string = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive();
                    auto size = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive();
                    auto result = syscall(SYS_write, file_descriptor.getI32(), string.getString().c_str(), size.getU64());
                    return PrimitiveValue(result < 0? -errno: result);

                #else
                    return PrimitiveValue::invalidValue;
                #endif
                }
                else if(name == "sys_exit")
                {
                #ifdef LINC_LINUX                    
                    auto arg_0 = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive();
                    auto result = syscall(SYS_exit, arg_0.getI32());
                    return PrimitiveValue(result < 0? -errno: result);
                #else
                    return PrimitiveValue::invalidValue;
                #endif
                }
                else if(name == "sys_open")
                {
                #ifdef LINC_LINUX
                    auto filename = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString();
                    auto flags = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive().getI32();
                    auto mode = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive().getU16();

                    auto result = syscall(SYS_open, filename.c_str(), flags, mode);
                    return PrimitiveValue(result < 0? -errno: result);
                #else
                    return PrimitiveValue::invalidValue;
                #endif
                }
                else if(name == "sys_read")
                {
                #ifdef LINC_LINUX
                    auto identifier = dynamic_cast<const BoundIdentifierExpression*>(external_call->getArguments().at(1ul).get());
                    if(!identifier)
                        return (Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = "String argument in sys_read must be an identifier"
                        }), PrimitiveValue::invalidValue);

                    auto file = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getI32();
                    auto buffer = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive().getString();
                    auto count = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive().getU64();
                    buffer.resize(count);

                    auto result = syscall(SYS_read, file, &buffer[0ul], count);

                    *m_identifiers.get(identifier->getValue()) = PrimitiveValue(buffer);
                    return PrimitiveValue(result < 0? -errno: result);
                #else
                    return PrimitiveValue::invalidValue;
                #endif
                }
                else if(name == "sys_close")
                {
                #ifdef LINC_LINUX
                    auto file_descriptor = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getI32();
                    auto result = syscall(SYS_close, file_descriptor);
                    return PrimitiveValue(result < 0? -errno: result);
                #else
                    return PrimitiveValue::invalidValue;
                #endif
                }
                else if(name == "system")
                {
                    class Deleter
                    {
                    public:
                        inline void operator()(std::FILE* file){ pclose(file); }
                    };

                    static std::array<char, 128ul> buffer;
                    auto argument = evaluateExpression(external_call->getArguments().at(0ul).get());

                    std::string result, command = argument.getPrimitive().getString();

                    std::unique_ptr<std::FILE, Deleter> pipe(popen(command.c_str(), "r"));
                    
                    if(!pipe)
                        return (Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = Logger::format("Failed to open pipe while evaluating shell expression (with command '$').", command)
                        }), PrimitiveValue::invalidValue);

                    while(std::fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
                    return PrimitiveValue(result);
                }
                
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("Internal function `$` has not been implemented.", name)
                });

                return PrimitiveValue::invalidValue;
            }
            else if(auto conversion_expression = dynamic_cast<const BoundConversionExpression*>(expression))
            {
                auto value = evaluateExpression(conversion_expression->getExpression());
                return value.getPrimitive().convert(conversion_expression->getType().primitive);
            }
            else if(auto array_initializer_expression = dynamic_cast<const BoundArrayInitializerExpression*>(expression))
            {
                ArrayValue result = ArrayValue::fromDefault(*expression->getType().array.baseType, array_initializer_expression->getValues().size());

                for(std::size_t i{0ul}; i < array_initializer_expression->getValues().size(); ++i)
                    result.set(i, evaluateExpression(array_initializer_expression->getValues()[i].get()));

                return std::move(result);
            }
            else if(auto structure_initializer_expression = dynamic_cast<const BoundStructureInitializerExpression*>(expression))
            {
                std::vector<Value> values;

                for(const auto& value: structure_initializer_expression->getFields())
                    values.push_back(evaluateExpression(value.get()));

                return Value(std::move(values));
            }
            else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
            {
                auto array = evaluateExpression(index_expression->getArray());
                auto index = evaluateExpression(index_expression->getIndex());
                auto type = index_expression->getArray()->getType();

                if(type.kind == Types::type::Kind::Primitive && type.primitive == Types::Kind::string)
                    return PrimitiveValue(array.getPrimitive().getString().at(index.getPrimitive().getU64()));

                else if(type.kind == Types::type::Kind::Array)
                    return array.getArray().get(index.getPrimitive().getU64());
                
                else return PrimitiveValue::invalidValue;
            }
            else if(auto access_expression = dynamic_cast<const BoundAccessExpression*>(expression))
            {
                auto base = evaluateExpression(access_expression->getBase());
                auto index = access_expression->getIndex();

                if(!base.getIfStructure())
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Info, .stage = Reporting::Stage::Generator,
                        .message = "Tried to evaluate access expression on non structure operand."
                    }), PrimitiveValue::invalidValue);

                if(index >= base.getStructure().size())
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Info, .stage = Reporting::Stage::Generator,
                        .message = "Tried to access value outside of array's bounds."
                    }), base.getStructure().empty()? Value(PrimitiveValue::invalidValue): base.getStructure().at(0ul));

                return base.getStructure().at(index);
            }
            else if(auto enumerator_expression = dynamic_cast<const BoundEnumeratorExpression*>(expression))
            {
                auto index = enumerator_expression->getEnumeratorIndex();
                auto value = enumerator_expression->getValue()? evaluateExpression(enumerator_expression->getValue()): PrimitiveValue::voidValue;
                auto name = m_enumerations.get(enumerator_expression->getEnumerationName())->at(enumerator_expression->getEnumeratorIndex()).first;

                return EnumeratorValue(name, index, std::move(value));
            }
            else if(auto range_expression = dynamic_cast<const BoundRangeExpression*>(expression))
            {
                auto begin = evaluateExpression(range_expression->getBegin());
                auto end = evaluateExpression(range_expression->getEnd());
                return Value(std::vector<Value>{begin, end, PrimitiveValue{false}});
            }
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized expression type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        inline void reset()
        {
            m_identifiers = ScopeStack<Value>{};
        }

        static void printNodeTree(const BoundNode* node, std::string indent = "", bool last = true)
        {
            auto marker = last? "└──" : "├──";
            
            if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(node))
            {
                printNodeTree(expression_statement->getExpression(), indent, last);
                return;
            }
            else if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(node))
            {
                printNodeTree(declaration_statement->getDeclaration(), indent, last);
                return;
            }

            Logger::println("$:$:$", indent, marker, node->toString());
            indent += last? "   ": "│  ";

            const BoundNode* const last_child = node->getChildren().size() > 0ul? node->getChildren()[node->getChildren().size() - 1ul]: nullptr;

            for(auto child: node->getChildren())
                printNodeTree(child, indent, child == last_child);
        }
    private:
        Value evaluateMutableOperator(const Types::type& type, const BoundExpression* expression, const Value& new_value)
        {
            Value result = Value::fromDefault(type);

            if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(expression))
            {
                *m_identifiers.get(identifier->getValue()) = new_value;
                result = new_value;
            }
            else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
            {
                auto index = evaluateExpression(index_expression->getIndex()).getPrimitive().getU64();
                auto array = index_expression->getArray();

                if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(array))
                {
                    auto find = m_identifiers.find(identifier->getValue());
                    
                    if(!find)
                        return PrimitiveValue::invalidValue;

                    find->getArray().set(index, new_value);
                    result = new_value;
                    return result;
                }
                
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = "Cannot use mutable operator on temporary array operands."
                });

                return PrimitiveValue::invalidValue;
            }
            else if(auto access_expression = dynamic_cast<const BoundAccessExpression*>(expression))
            {
                auto index = access_expression->getIndex();
                auto base = access_expression->getBase();

                if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(base))
                {
                    auto find = m_identifiers.find(identifier->getValue());
                    
                    if(!find)
                        return PrimitiveValue::invalidValue;

                    find->getStructure().at(index) = new_value;
                    result = new_value;
                    return result;
                }

                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = "Cannot use mutable operator on temporary structure operands."
                });

                return PrimitiveValue::invalidValue;
            }
            else return (Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = "Cannot use mutable operator on temporary operands."
                }), PrimitiveValue::invalidValue);

            if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                return result.getPrimitive().convert(type.primitive);
            else return result;
        }

        void beginScope()
        {
            m_identifiers.beginScope();
            m_enumerations.beginScope();
        }

        void endScope()
        {
            m_identifiers.endScope();
            m_enumerations.endScope();
        }

        ScopeStack<Value> m_identifiers;
        ScopeStack<Types::type::Enumeration> m_enumerations;
    };
}
