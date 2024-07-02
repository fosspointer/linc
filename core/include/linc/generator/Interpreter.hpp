#pragma once
#include <linc/Tree.hpp>
#include <linc/BoundTree.hpp>
#include <linc/Include.hpp>
#include <linc/Binder.hpp>

#define LINC_EXIT_PROGRAM_FAILURE 5
#define LINC_EXIT_PROGRAM_SUCCESS 0

namespace linc
{
    class Interpreter final
    {
    public:
        struct VariableValue final
        {   
            std::string name;
            Value value{PrimitiveValue::invalidValue};
            bool isMutable;
            bool temporary{false};
        };

        [[nodiscard("The return value of this function must match that of the environment's entry point (i.e. main()).")]]
        int evaluateProgram(const BoundProgram* program, Binder& binder, std::unique_ptr<const ArrayInitializerExpression> argument_list)
        {
            for(const auto& declaration: program->declarations)
                evaluateDeclaration(declaration.get());
        
            auto main_name = linc::PrimitiveValue(std::string{"main"});
            auto find_main = binder.find(main_name.getString());
            if(find_main == binder.end())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("Call to undeclared entry-point function $.", main_name)
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }
            else if(auto main = dynamic_cast<const BoundFunctionDeclaration*>(find_main->get()); !main)
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = Logger::format("The symbol-name $ is reserved for the entry point function, and cannot be used for other symbol types.",
                        main_name)
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }

            auto main = static_cast<const BoundFunctionDeclaration*>(find_main->get());
            auto main_argument_list = std::vector<CallExpression::Argument>{};
            
            if(!main->getArguments().empty())
                main_argument_list.push_back(CallExpression::Argument{std::nullopt, std::move(argument_list)});

            auto main_call = std::make_unique<const linc::CallExpression>(
                linc::Token{.type = linc::Token::Type::Identifier, .value = main->getName()},
                linc::Token{.type = linc::Token::Type::ParenthesisLeft},
                linc::Token{.type = linc::Token::Type::ParenthesisRight},
                std::move(main_argument_list), false);

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

        Value evaluateStatement(const BoundStatement* statement)
        {
            if(m_returnValue.has_value())
                return m_returnValue.value();
                
            if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                return evaluateDeclaration(declaration_statement->getDeclaration());
            
            else if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
                return evaluateExpression(expression_statement->getExpression());
            
            else if(auto label_statement = dynamic_cast<const BoundLabelStatement*>(statement))
                return evaluateStatement(label_statement->getNext());

            else if(auto jump_statement = dynamic_cast<const BoundJumpStatement*>(statement))
            {
                m_jumpIndex = jump_statement->getBlockIndex();
                m_jumpScope = jump_statement->getScope();
                return PrimitiveValue::voidValue;
            }

            else if(auto return_statement = dynamic_cast<const BoundReturnStatement*>(statement))
            {
                m_returnValue = evaluateExpression(return_statement->getExpression());
                return m_returnValue.value();
            }

            else if(auto break_statement = dynamic_cast<const BoundBreakStatement*>(statement))
            {
                m_breakScope = break_statement->getScope();
                return PrimitiveValue::voidValue;
            }

            else if(auto continue_statement = dynamic_cast<const BoundContinueStatement*>(statement))
            {
                m_continueScope = continue_statement->getScope();
                return PrimitiveValue::voidValue;
            }

            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized statement type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        Value evaluateDeclaration(const BoundDeclaration* declaration)
        {
            if(m_returnValue.has_value())
                return m_returnValue.value();
                
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            {
                auto value = variable_declaration->getDefaultValue()? evaluateExpression(*variable_declaration->getDefaultValue()):
                    Value::fromDefault(variable_declaration->getActualType());
                auto variable = m_variables[variable_declaration->getName()] 
                    = VariableValue{.name = variable_declaration->getName(), .value = value, .isMutable = variable_declaration->getActualType().isMutable};

                return PrimitiveValue::voidValue;
            }
            else if(auto function_declaration = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
            else if(auto external_declaration = dynamic_cast<const BoundExternalDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
            else if(auto structure_declaration = dynamic_cast<const BoundStructureDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized declaration type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        Value evaluateExpression(const BoundExpression* expression)
        {
            if(m_returnValue.has_value())
                return m_returnValue.value();
                
            if(auto literal_expression = dynamic_cast<const BoundLiteralExpression*>(expression))
            {
                return literal_expression->getValue();
            }
            else if(auto block_expression = dynamic_cast<const BoundBlockExpression*>(expression))
            {
                Value value = PrimitiveValue::voidValue; 
                m_returnValue = std::nullopt;
                ++m_scope;
                for(std::size_t i{0ul}; i < block_expression->getStatements().size(); ++i)
                {
                    if(m_jumpIndex != -1ul && m_scope == m_jumpScope)
                    {
                        i = m_jumpIndex -1ul;
                        m_jumpIndex = m_jumpScope = -1ul;
                        continue;
                    }
                    else if(m_scope == m_breakScope)
                    {
                        --m_breakScope;
                        break;
                    }
                    else if(m_scope == m_continueScope)
                    {
                        --m_continueScope;
                        break;
                    }
                    else if(m_scope > m_breakScope || m_scope > m_continueScope)
                        break;
                        
                    if(m_returnValue.has_value())
                        break;

                    const auto& stmt = block_expression->getStatements()[i];
                    value = evaluateStatement(stmt.get());

                }
                --m_scope;
                
                return m_returnValue.has_value()? m_returnValue.value(): value; 
            }
            else if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
            {
                auto test = evaluateExpression(if_expression->getTestExpression()).getPrimitive().getBool();

                if(test)
                    return evaluateStatement(if_expression->getIfBodyStatement());
                else if(if_expression->hasElse())
                    return evaluateStatement(if_expression->getElseBodyStatement().value());
                else return PrimitiveValue::voidValue;
            }
            else if(auto for_expression = dynamic_cast<const BoundForExpression*>(expression))
            {
                const auto& specifier = for_expression->getSpecifier();

                ++m_scope;
                if(auto variable_specifier = std::get_if<const BoundForExpression::BoundVariableForSpecifier>(&specifier))
                {
                    evaluateDeclaration(variable_specifier->variableDeclaration.get());
                    Value return_value = PrimitiveValue::voidValue;

                    while(!m_returnValue.has_value() && evaluateExpression(variable_specifier->expression.get()).getPrimitive().getBool())
                    {
                        if(m_scope == m_breakScope)
                        {
                            m_breakScope = -1ul;
                            break;
                        }
                        else if(m_scope == m_continueScope)
                        {
                            m_continueScope = -1ul;
                            continue;
                        }
                        else if(m_scope > m_breakScope || m_scope > m_continueScope)
                            break;

                        return_value = evaluateStatement(for_expression->getBody());
                        evaluateStatement(variable_specifier->statement.get());
                    }
                    
                    return (--m_scope, return_value);
                }
                else if(auto range_specifier = std::get_if<const BoundForExpression::BoundRangeForSpecifier>(&specifier))
                {
                    auto find = m_variables.find(range_specifier->arrayIdentifier->getValue());
                    auto type = range_specifier->arrayIdentifier->getType();
                    std::size_t count{};
                    ArrayValue array(Types::voidType, 0ul);

                    if(type.kind == Types::type::Kind::Primitive && type.primitive == Types::Kind::string)
                    {
                        auto string = find->second.value.getPrimitive().getString();

                        count = string.size();
                        array = ArrayValue(std::vector<char>(string.c_str(), string.c_str() + count));
                    }
                    else if(type.kind == Types::type::Kind::Array)
                    {
                        count = find->second.value.getArray().getCount();
                        array = find->second.value.getArray();
                    }
                    else return (--m_scope, PrimitiveValue::invalidValue);
                    Value return_value{PrimitiveValue::voidValue};
                    
                    for(std::size_t i{0ul}; i < count; ++i)
                    {
                        m_variables[range_specifier->valueIdentifier->getValue()] = VariableValue{
                            .name = range_specifier->valueIdentifier->getValue(),
                            .value = array.get(i),
                            .isMutable = true
                        };

                        return_value = evaluateStatement(for_expression->getBody());
                    }

                    return (--m_scope, return_value);
                }
                else return (--m_scope, PrimitiveValue::invalidValue);
            }
            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
            {
                Value return_value = PrimitiveValue::voidValue;
                bool evaluated{false}, can_continue{true};

                ++m_scope;
                if(evaluateExpression(while_expression->getTestExpression()).getPrimitive().getBool())
                {
                    return_value = evaluateStatement(while_expression->getWhileBodyStatement());

                    evaluated = true;
                    if(m_scope == m_breakScope) { m_breakScope = -1ul; can_continue = false; }
                }

                while(evaluated && can_continue && !m_returnValue.has_value() && evaluateExpression(while_expression->getTestExpression()).getPrimitive().getBool())
                {
                    if(m_scope == m_breakScope)
                    {
                        m_breakScope = -1ul;
                        break;
                    }
                    else if(m_scope == m_continueScope)
                    {
                        m_continueScope = -1ul;
                        continue;
                    }
                    else if(m_scope > m_breakScope || m_scope > m_continueScope)
                        break;

                    return_value = evaluateStatement(while_expression->getWhileBodyStatement());
                }
                
                auto finally = while_expression->getFinallyBodyStatement();
                auto _else = while_expression->getElseBodyStatement();

                if(evaluated && finally.has_value())
                    return_value = evaluateStatement(finally.value());
                else if(!evaluated && _else.has_value())
                    return_value = evaluateStatement(_else.value());

                return (--m_scope, return_value);
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
                Value result = PrimitiveValue::fromDefault(unary_expression->getType().primitive);
                
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
                    else result = operand;
                    break;
                case BoundUnaryOperator::Kind::UnaryMinus:
                    result = -operand;
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
                auto find = m_variables.find(identifier_expression->getValue());
                
                if(find == m_variables.end())
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Variable '$' does not exist!", identifier_expression->getValue())
                    }), PrimitiveValue::invalidValue);

                return find->second.value;
            }
            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
            {
                return PrimitiveValue(type_expression->getActualType());
            }
            else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
            {
                std::vector<std::string> args;
                m_returnValue = std::nullopt;

                ++m_scope;
                for(const auto& argument: function_call_expression->getArguments())
                {
                    auto find = m_variables.find(argument.name);

                    auto value = evaluateExpression(argument.value.get());
                    auto variable = m_variables[argument.name] 
                        = VariableValue{.name = argument.name, .value = value, .isMutable = argument.value->getType().isMutable,
                            .temporary = find == m_variables.end()};
                    args.push_back(variable.name);
                }

                auto result = evaluateStatement(function_call_expression->getBody());

                for(const auto& argument: function_call_expression->getArguments())
                {
                    auto find = m_variables.find(argument.name);
                    if(find != m_variables.end() && find->second.temporary)
                        m_variables.erase(find);
                }

                return (--m_scope, result);
            }
            else if(auto external_call = dynamic_cast<const BoundExternalCallExpression*>(expression))
            {
                const auto& name = external_call->getName();
                
                if(name == "puts")
                {
                    fputs(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString().c_str(), stdout);
                    return PrimitiveValue::voidValue;
                }
                else if(name == "putc")
                {
                    fputc(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getChar(), stdout);
                    return PrimitiveValue::voidValue;
                }
                else if(name == "readchar")
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
                else if(name == "system")
                {
                    class Deleter
                    {
                    public:
                        inline void operator()(std::FILE* file){ pclose(file); }
                    };

                    static std::array<char, 128> buffer;
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
                ArrayValue result = ArrayValue::fromDefault(*expression->getType().array.base_type);

                for(const auto& value: array_initializer_expression->getValues())
                    result.push(evaluateExpression(value.get()));

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
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized expression type while evaluating program"); 
                return PrimitiveValue::invalidValue;
            }
        }

        inline void reset()
        {
            m_variables.clear();
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
                m_variables.at(identifier->getValue()).value = new_value;
                result = new_value;
            }
            else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
            {
                auto index = evaluateExpression(index_expression->getIndex()).getPrimitive().getU64();
                auto array = index_expression->getArray();

                if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(array))
                {
                    auto find = m_variables.find(identifier->getValue());
                    
                    if(find == m_variables.end())
                        return PrimitiveValue::invalidValue;

                    find->second.value.getArray().set(index, new_value);
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
                    auto find = m_variables.find(identifier->getValue());
                    
                    if(find == m_variables.end())
                        return PrimitiveValue::invalidValue;

                    find->second.value.getStructure().at(index) = new_value;
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

        Types::u64 m_scope{0ul}, m_jumpIndex{-1ul}, m_jumpScope{-1ul}, m_breakScope{-1ul}, m_continueScope{-1ul};
        std::unordered_map<std::string, VariableValue> m_variables;
        std::optional<Value> m_returnValue{std::nullopt};
    };
}
