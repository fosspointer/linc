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

        struct Function final
        {
            const BoundFunctionDeclaration* function;
        };

        [[nodiscard("The return type of this function is to be returned by main()")]]
        int evaluateProgram(const BoundProgram* program, Binder& binder, std::unique_ptr<const ArrayInitializerExpression> argument_list)
        {
            for(const auto& declaration: program->declarations)
                evaluateDeclaration(declaration.get());
        
            auto find_main = binder.find("main");
            if(find_main == binder.end())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = "Cannot evaluate program, for which the entry-point is not defined. Have you tried defining the main() function?"
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }
            else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(find_main->get()))
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                    .message = "The symbol-name 'main' is reserved for the entry point function, and cannot be used elsewhere."
                });
                return LINC_EXIT_PROGRAM_FAILURE;
            }

            auto main = dynamic_cast<const BoundFunctionDeclaration*>(find_main->get());
            auto main_argument_list = std::vector<std::unique_ptr<const Expression>>{};
            
            if(!main->getArguments().empty())
                main_argument_list.push_back(std::move(argument_list));

            auto main_call = std::make_unique<const linc::FunctionCallExpression>(
                linc::Token{.type = linc::Token::Type::Identifier, .value = main->getName()},
                linc::Token{.type = linc::Token::Type::ParenthesisLeft},
                linc::Token{.type = linc::Token::Type::ParenthesisRight},
                std::move(main_argument_list));

            auto bound_main_call = binder.bindExpression(main_call.get());

            bool errors{false};
            for(const auto& report: Reporting::getReports())
                if(report.type == Reporting::Type::Error)
                    errors = true;

            if(errors)
                return LINC_EXIT_PROGRAM_FAILURE;

            switch(bound_main_call->getType().kind)
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
                        Types::toString(bound_main_call->getType()))
                }});
                return LINC_EXIT_PROGRAM_FAILURE;
            }
        }

        Value evaluateStatement(const BoundStatement* statement)
        {
            if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                return evaluateDeclaration(declaration_statement->getDeclaration());
            
            else if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
                return evaluateExpression(expression_statement->getExpression());
            
            else if(auto scope_statement = dynamic_cast<const BoundScopeStatement*>(statement))
            {
                Value value = PrimitiveValue::voidValue;
                for(const auto& stmt: scope_statement->getStatements())
                    value = evaluateStatement(stmt.get());
                
                return value; 
            }
            else if(auto put_character_statement = dynamic_cast<const BoundPutCharacterStatement*>(statement))
            {
                fputc(evaluateExpression(put_character_statement->getExpression()).getPrimitive().getChar(), stdout);
                return PrimitiveValue::voidValue;
            }
            else if(auto put_string_statement = dynamic_cast<const BoundPutStringStatement*>(statement))
            {
                fputs(evaluateExpression(put_string_statement->getExpression()).getPrimitive().getString().c_str(), stdout);
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
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            {
                auto value = variable_declaration->getDefaultValue()? evaluateExpression(*variable_declaration->getDefaultValue()):
                    PrimitiveValue::fromDefault(variable_declaration->getActualType().kind);
                auto variable = m_variables[variable_declaration->getName()] 
                    = VariableValue{.name = variable_declaration->getName(), .value = value, .isMutable = variable_declaration->getActualType().isMutable};

                return PrimitiveValue::voidValue;
            }
            else if(auto function_declaration = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
                return PrimitiveValue::voidValue;
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
            else if(auto if_else_expression = dynamic_cast<const BoundIfElseExpression*>(expression))
            {
                auto test = evaluateExpression(if_else_expression->getTestExpression()).getPrimitive().getBool();

                if(test)
                    return evaluateStatement(if_else_expression->getIfBodyStatement());
                else if(if_else_expression->hasElse())
                    return evaluateStatement(if_else_expression->getElseBodyStatement().value());
                else return PrimitiveValue::voidValue;
            }
            else if(auto for_expression = dynamic_cast<const BoundForExpression*>(expression))
            {
                const auto& specifier = for_expression->getSpecifier();

                if(auto variable_specifier = std::get_if<const BoundForExpression::BoundVariableForSpecifier>(&specifier))
                {
                    evaluateDeclaration(variable_specifier->variableDeclaration.get());
                    Value return_value = PrimitiveValue::voidValue;

                    while(evaluateExpression(variable_specifier->expression.get()).getPrimitive().getBool())
                    {
                        return_value = evaluateStatement(for_expression->getBody());
                        evaluateStatement(variable_specifier->statement.get());
                    }

                    return return_value;
                }
                else if(auto range_specifier = std::get_if<const BoundForExpression::BoundRangeForSpecifier>(&specifier))
                {
                    auto find = m_variables.find(range_specifier->arrayIdentifier->getValue());
                    auto type = range_specifier->arrayIdentifier->getType();
                    std::size_t count{};
                    ArrayValue array(std::vector<Types::_void_type>{Types::_void_type{}});

                    if(!type.isArray && type.kind == Types::Kind::string)
                    {
                        auto string = find->second.value.getPrimitive().getString();

                        count = string.size();
                        array = ArrayValue(std::vector<char>(string.c_str(), string.c_str() + count));
                    }
                    else if(type.isArray)
                    {
                        count = find->second.value.getArray().getCount();
                        array = find->second.value.getArray();
                    }
                    else return PrimitiveValue::invalidValue;

                    Value return_value{PrimitiveValue::voidValue};
                    
                    for(std::size_t i = 0ull; i < count; ++i)
                    {
                        m_variables[range_specifier->valueIdentifier->getValue()] = VariableValue{
                            .name = range_specifier->valueIdentifier->getValue(),
                            .value = PrimitiveValue(array.get(i)),
                            .isMutable = true
                        };

                        return_value = evaluateStatement(for_expression->getBody());
                    }

                    return return_value;
                }
                else return PrimitiveValue::invalidValue;
            }
            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
            {
                Value return_value = PrimitiveValue::voidValue;
                bool evaluated{false};

                if(evaluateExpression(while_expression->getTestExpression()).getPrimitive().getBool())
                {
                    evaluated = true;
                    return_value = evaluateStatement(while_expression->getWhileBodyStatement());
                }

                while(evaluateExpression(while_expression->getTestExpression()).getPrimitive().getBool())
                    return_value = evaluateStatement(while_expression->getWhileBodyStatement());
                
                auto finally = while_expression->getFinallyBodyStatement();
                auto _else = while_expression->getElseBodyStatement();

                if(evaluated && finally.has_value())
                    return_value = evaluateStatement(finally.value());
                else if(!evaluated && _else.has_value())
                    return_value = evaluateStatement(_else.value());

                return return_value;
            }
            else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
            {
                Value result = PrimitiveValue::fromDefault(binary_expression->getType().kind);
                
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

                auto left = evaluateExpression(binary_expression->getLeft()).getPrimitive();
                auto right = evaluateExpression(binary_expression->getRight()).getPrimitive();

                switch(binary_expression->getOperator()->getKind())
                {
                case BoundBinaryOperator::Kind::Assignment:
                    if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(binary_expression->getLeft()))
                    {
                        m_variables.at(identifier->getValue()).value = right;
                        result = right;
                    }
                    else if(auto array_index = dynamic_cast<const BoundArrayIndexExpression*>(binary_expression->getLeft()))
                    {
                        auto index = evaluateExpression(array_index->getIndex()).getPrimitive().getU64();
                        auto find = m_variables.find(array_index->getIdentifier()->getValue());

                        if(find == m_variables.end())
                            return PrimitiveValue::invalidValue;

                        find->second.value.getArray().set(index, right);
                        result = right;
                    }
                    else
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = "Cannot use the assignment operator on temporary operands."
                        });
                        return PrimitiveValue::invalidValue;
                    }
                    break;
                case BoundBinaryOperator::Kind::Subtraction:
                    result = left - right;
                    break;
                case BoundBinaryOperator::Kind::Multiplication:
                    result = left * right;
                    break;
                case BoundBinaryOperator::Kind::Division:
                    if(right.isZero())
                    {
                        Reporting::push({Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = Logger::format("Attempted division by zero. Operands are '$' ('$') and '$' ('$').",
                                left, Types::toString(left.getType()), right, Types::toString(right.getType()))
                        }});
                        return PrimitiveValue::invalidValue;
                    }
                    result = left / right;
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
                default: break;
                }

                if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                    || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                    return result.getPrimitive().convert(binary_expression->getType().kind);
                else return result;
            }
            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
            {
                Value result = PrimitiveValue::fromDefault(unary_expression->getType().kind);
                
                if(unary_expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Typeof)
                    return PrimitiveValue(unary_expression->getOperand()->getType());

                auto operand = evaluateExpression(unary_expression->getOperand());

                switch(unary_expression->getOperator()->getKind())
                {
                case BoundUnaryOperator::Kind::Increment:
                case BoundUnaryOperator::Kind::Decrement:
                {
                    const BoundIdentifierExpression* operand;
                    if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(unary_expression->getOperand()))
                    {
                        auto var = m_variables.find(identifier->getValue());

                        if(var == m_variables.end())
                            return PrimitiveValue::invalidValue;
                        else
                        {
                            if(!var->second.isMutable)
                            {
                                Reporting::push(Reporting::Report{
                                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                                    .message = Logger::format("Cannot increment/decrement immutable variable '$'.",
                                        identifier->getValue())
                                });

                                return PrimitiveValue::invalidValue;
                            }
                            else 
                            {
                                if(unary_expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Increment)
                                    return Value(++var->second.value).getPrimitive().convert(unary_expression->getType().kind);
                                else return Value(--var->second.value).getPrimitive().convert(unary_expression->getType().kind);
                            }
                        }
                    }
                    else
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = "Cannot increment non-identifier expression"
                        });

                        return PrimitiveValue::invalidValue;
                    }
                }
                case BoundUnaryOperator::Kind::Stringify:
                    return PrimitiveValue(operand.toString());
                case BoundUnaryOperator::Kind::UnaryPlus:
                    if(operand.getIfArray())
                        return PrimitiveValue(operand.getArray().getCount());
                    else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::String)
                        return PrimitiveValue(static_cast<Types::u64>(operand.getPrimitive().getString().size()));
                    else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::Character)
                        return PrimitiveValue(static_cast<Types::i32>(operand.getPrimitive().getChar()));
                    else return operand;
                case BoundUnaryOperator::Kind::UnaryMinus:
                    return -operand;
                case BoundUnaryOperator::Kind::LogicalNot:
                    return PrimitiveValue(!operand.getPrimitive().getBool());
                default: return PrimitiveValue::invalidValue;
                }

                if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                    || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                    return result.getPrimitive().convert(binary_expression->getType().kind);
                else return result;
            }
            else if(auto identifier_expression = dynamic_cast<const BoundIdentifierExpression*>(expression))
            {
                auto find = m_variables.find(identifier_expression->getValue());
                
                if(find == m_variables.end())
                {
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Variable '$' does not exist!", identifier_expression->getValue())
                    });
                    return PrimitiveValue::invalidValue;
                }

                return find->second.value;
            }
            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
            {
                return PrimitiveValue(type_expression->getActualType());
            }
            else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
            {
                std::vector<std::string> args;

                for(const auto& argument: function_call_expression->getArguments())
                {
                    auto find = m_variables.find(argument.name);

                    auto value = evaluateExpression(argument.value.get());
                    auto variable = m_variables[argument.name] 
                        = VariableValue{.name = argument.name, .value = value, .isMutable = argument.isMutable, .temporary = find == m_variables.end()};
                    args.push_back(variable.name);
                }

                auto result = evaluateStatement(function_call_expression->getBody());

                for(const auto& argument: function_call_expression->getArguments())
                {
                    auto find = m_variables.find(argument.name);
                    if(find != m_variables.end() && find->second.temporary)
                        m_variables.erase(find);
                }

                return result;
            }
            else if(auto conversion_expression = dynamic_cast<const BoundConversionExpression*>(expression))
            {
                auto value = evaluateExpression(conversion_expression->getExpression());
                return value.getPrimitive().convert(conversion_expression->getType().kind);
            }
            else if(auto array_initializer_expression = dynamic_cast<const BoundArrayInitializerExpression*>(expression))
            {
                ArrayValue result = ArrayValue::fromDefault(expression->getType().kind);

                for(const auto& value: array_initializer_expression->getValues())
                    result.push(evaluateExpression(value.get()).getPrimitive());

                return std::move(result);
            }
            else if(auto array_index_expression = dynamic_cast<const BoundArrayIndexExpression*>(expression))
            {
                auto find = m_variables.find(array_index_expression->getIdentifier()->getValue());
                auto index = evaluateExpression(array_index_expression->getIndex());
                auto type = array_index_expression->getIdentifier()->getType();

                if(type.kind == Types::Kind::string && !type.isArray)
                    return PrimitiveValue(find->second.value.getPrimitive().getString().at(index.getPrimitive().getU64()));

                else if(type.isArray)
                    return find->second.value.getArray().get(index.getPrimitive().getU64());
                
                else return PrimitiveValue::invalidValue;
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

        static void printNodeTree(Binder binder, const Node* node, std::string indent = "", bool last = true)
        {
            auto marker = last? "└──" : "├──";

            binder.reset();
            Logger::println("$:$:$", indent, marker, binder.bindNode(node)->toString());

            indent += last? "   ": "│  ";

            const Node* last_child = node->getChildren().size() > 0? node->getChildren()[node->getChildren().size() - 1]: nullptr;

            for(auto child: node->getChildren())
            {
                printNodeTree(binder, child, indent, child == last_child);
            }
        }
    private:
        std::unordered_map<std::string, VariableValue> m_variables;
        std::unordered_map<std::string, VariableValue> m_functions;
    };
}