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
            TypedValue value{TypedValue::invalidValue};
            bool isMutable;
            bool temporary{false};
        };

        struct Function final
        {
            const BoundFunctionDeclaration* function;
        };

        [[nodiscard("The return type of this function is to be returned by main()")]]
        int evaluateProgram(const BoundProgram* program, Binder& binder)
        {
            for(const auto& declaration: program->declarations)
                evaluateDeclaration(declaration.get());
        
            auto main_call = std::make_unique<const linc::FunctionCallExpression>(
                linc::Token{.type = linc::Token::Type::Identifier, .value = "main"},
                linc::Token{.type = linc::Token::Type::ParenthesisLeft},
                linc::Token{.type = linc::Token::Type::ParenthesisRight},
                std::vector<std::unique_ptr<const linc::Expression>>{});

            auto bound_main_call = binder.bindExpression(main_call.get());

            bool errors{false};
            for(const auto& report: Reporting::getReports())
                if(report.type == Reporting::Type::Error)
                    errors = true;

            if(errors)
                return LINC_EXIT_PROGRAM_FAILURE;

            switch(bound_main_call->getType())
            {
            case Types::Type::u8: return evaluateExpression(bound_main_call.get()).getU8();
            case Types::Type::i8: return evaluateExpression(bound_main_call.get()).getI8();
            case Types::Type::i16: return evaluateExpression(bound_main_call.get()).getI16();
            case Types::Type::i32: return evaluateExpression(bound_main_call.get()).getI32();
            case Types::Type::_void:
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

        TypedValue evaluateStatement(const BoundStatement* statement)
        {
            if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                return evaluateDeclaration(declaration_statement->getDeclaration());
            
            else if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
                return evaluateExpression(expression_statement->getExpression());
            
            else if(auto scope_statement = dynamic_cast<const BoundScopeStatement*>(statement))
            {
                TypedValue value = TypedValue::voidValue;
                for(const auto& stmt: scope_statement->getStatements())
                    value = evaluateStatement(stmt.get());
                
                return value; 
            }
            else if(auto put_character_statement = dynamic_cast<const BoundPutCharacterStatement*>(statement))
            {
                fputc(evaluateExpression(put_character_statement->getExpression()).getChar(), stdout);
                return TypedValue::voidValue;
            }
            else if(auto put_string_statement = dynamic_cast<const BoundPutStringStatement*>(statement))
            {
                fputs(evaluateExpression(put_string_statement->getExpression()).getString().c_str(), stdout);
                return TypedValue::voidValue;
            }
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized statement type while evaluating program"); 
                return TypedValue::invalidValue;
            }
        }

        TypedValue evaluateDeclaration(const BoundDeclaration* declaration)
        {
            if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            {
                auto value = evaluateExpression(variable_declaration->getValueExpression());
                auto variable = m_variables[variable_declaration->getName()] 
                    = VariableValue{.name = variable_declaration->getName(), .value = value, .isMutable = variable_declaration->getMutable()};

                return TypedValue::voidValue;
            }
            else if(auto function_declaration = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
                return TypedValue::voidValue;
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized declaration type while evaluating program"); 
                return TypedValue::invalidValue;
            }
        }

        TypedValue evaluateExpression(const BoundExpression* expression)
        {
            if(auto literal_expression = dynamic_cast<const BoundLiteralExpression*>(expression))
            {
                return literal_expression->getValue();
            }
            else if(auto if_else_expression = dynamic_cast<const BoundIfElseExpression*>(expression))
            {
                auto test = evaluateExpression(if_else_expression->getTestExpression()).getBool();

                if(test)
                    return evaluateStatement(if_else_expression->getIfBodyStatement());
                else if(if_else_expression->hasElse())
                    return evaluateStatement(if_else_expression->getElseBodyStatement().value());
                else return TypedValue::voidValue;
            }
            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
            {
                TypedValue return_value = TypedValue::voidValue;

                bool evaluated{false};

                if(evaluateExpression(while_expression->getTestExpression()).getBool())
                {
                    evaluated = true;
                    return_value = evaluateStatement(while_expression->getWhileBodyStatement());
                }

                while(evaluateExpression(while_expression->getTestExpression()).getBool())
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
                if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalAnd)
                {
                    if(evaluateExpression(binary_expression->getLeft()).getBool())
                        return TypedValue(evaluateExpression(binary_expression->getRight()).getBool());
                    else return TypedValue(false);
                }

                auto left = evaluateExpression(binary_expression->getLeft());
                auto right = evaluateExpression(binary_expression->getRight());

                switch(binary_expression->getOperator()->getKind())
                {
                case BoundBinaryOperator::Kind::Addition:
                    return left + right;
                case BoundBinaryOperator::Kind::Subtraction:
                    return left - right;
                case BoundBinaryOperator::Kind::Multiplication:
                    return left * right;
                case BoundBinaryOperator::Kind::Division:
                    if(right.isZero())
                    {
                        Reporting::push({Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = Logger::format("Attempted division by zero. Operands are '$' ('$') and '$' ('$').",
                                left, Types::toString(left.getType()), right, Types::toString(right.getType()))
                        }});
                        return TypedValue::invalidValue;
                    }
                    return left / right;
                case BoundBinaryOperator::Kind::LogicalOr:
                    return TypedValue(left.getBool() || right.getBool());
                case BoundBinaryOperator::Kind::Equals:
                    return TypedValue(left == right);
                case BoundBinaryOperator::Kind::NotEquals:
                    return TypedValue(left != right);
                case BoundBinaryOperator::Kind::Greater:
                    return TypedValue(left > right);
                case BoundBinaryOperator::Kind::Less:
                    return TypedValue(left < right);
                case BoundBinaryOperator::Kind::GreaterEqual:
                    return TypedValue(left >= right);
                case BoundBinaryOperator::Kind::LessEqual:
                    return TypedValue(left <= right);
                default: return TypedValue::invalidValue;
                }
            }
            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
            {
                auto operand = evaluateExpression(unary_expression->getOperand());

                switch(unary_expression->getOperator()->getKind())
                {
                case BoundUnaryOperator::Kind::Increment:
                case BoundUnaryOperator::Kind::Decrement:
                {
                    if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(unary_expression->getOperand()))
                    {
                        auto var = m_variables.find(identifier->getValue());

                        if(var == m_variables.end())
                            return TypedValue::invalidValue;
                        else
                        {
                            if(!var->second.isMutable)
                            {
                                Reporting::push(Reporting::Report{
                                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                                    .message = Logger::format("Cannot increment/decrement immutable variable '$'.",
                                        identifier->getValue())
                                });

                                return TypedValue::invalidValue;
                            }
                            else 
                            {
                                if(unary_expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Increment)
                                    return ++var->second.value;
                                else return --var->second.value;
                            }
                        }
                    }
                    else
                    {
                        Reporting::push(Reporting::Report{
                            .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                            .message = "Cannot increment non-identifier expression"
                        });

                        return TypedValue::invalidValue;
                    }
                }
                case BoundUnaryOperator::Kind::Stringify:
                    return TypedValue(operand.toString());
                case BoundUnaryOperator::Kind::UnaryPlus:
                    return operand;
                case BoundUnaryOperator::Kind::UnaryMinus:
                    return -operand;
                case BoundUnaryOperator::Kind::LogicalNot:
                    return TypedValue(!operand.getBool());
                default: return TypedValue::invalidValue;
                }
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
                    return TypedValue::invalidValue;
                }

                return find->second.value;
            }
            else if(auto variable_assignment_expression = dynamic_cast<const BoundVariableAssignmentExpression*>(expression))
            {
                auto find = m_variables.find(variable_assignment_expression->getIdentifier());
                find->second.value = evaluateExpression(variable_assignment_expression->getValue());
                return find->second.value;
            }
            else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
            {
                std::vector<std::string> args;

                for(const auto& argument: function_call_expression->getArguments())
                {
                    auto value = evaluateExpression(argument.value.get());
                    if(m_variables.find(argument.name) == m_variables.end())
                    {
                        auto variable = m_variables[argument.name] 
                            = VariableValue{.name = argument.name, .value = value, .isMutable = argument.isMutable, .temporary = true};
                        args.push_back(variable.name);
                    }
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
            else
            {
                throw LINC_EXCEPTION("Encountered unrecognized expression type while evaluating program"); 
                return TypedValue::invalidValue;
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