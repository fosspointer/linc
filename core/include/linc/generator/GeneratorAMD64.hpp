#pragma once
#include <linc/generator/Registers.hpp>
#include <linc/generator/Target.hpp>
#include <linc/generator/EmitterAMD64.hpp>

#define LINC_EXIT_PROGRAM_FAILURE 5
#define LINC_EXIT_PROGRAM_SUCCESS 0
#define LINC_GENERATORAMD64_STRING_LITERAL_TRUE "true"
#define LINC_GENERATORAMD64_STRING_LITERAL_FALSE "false"
#define LINC_GENERATORAMD64_STRING_LITERAL_VOID "{}"

namespace linc
{
    class GeneratorAMD64 final
    {
    public:
        using Emitter = EmitterAMD64;
        GeneratorAMD64(const BoundProgram* program, Target::Platform platform)
            :m_program(program), m_platform(platform)
        {}

        using Variable = std::variant<std::string, std::size_t>;

        std::pair<std::string, bool> generateProgram()
        {
            m_variables = ScopeStack<Variable>();
            m_variables.beginScope();
            m_hasMain = {};
            m_emitter.reset();

            for(const auto& declaration: m_program->declarations)
                generateDeclaration(declaration.get());

            m_variables.endScope();
            return std::pair<std::string, bool>{m_emitter.get(), m_hasMain};
        }

        void generateDeclaration(const BoundDeclaration* declaration)
        {
            if(auto function = dynamic_cast<const BoundFunctionDeclaration*>(declaration))
                generateFunctionDeclaration(function);

            else if(auto variable = dynamic_cast<const BoundVariableDeclaration*>(declaration))
                generateVariableDeclaration(variable);

            else if(auto external_function = dynamic_cast<const BoundExternalDeclaration*>(declaration))
                m_emitter.external(external_function->getName());

            else throw LINC_EXCEPTION("Declaration type not yet implemented.");
        }

        void generateStatement(const BoundStatement* statement)
        {
            if(auto expression_statement = dynamic_cast<const BoundExpressionStatement*>(statement))
            {
                generateExpression(expression_statement->getExpression());
                m_emitter.pop(Registers::getReturn());
            }

            else if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                generateDeclaration(declaration_statement->getDeclaration());

            else if(auto return_statement = dynamic_cast<const BoundReturnStatement*>(statement))
                generateReturnStatement(return_statement);

            else throw LINC_EXCEPTION("Statement type not yet implemented.");
        }

        void generateBlockExpression(const BoundBlockExpression* expression)
        {
            auto stack_position = m_emitter.getStackPosition();
            m_variables.beginScope();

            for(const auto& item: expression->getStatements())
                generateStatement(item.get());

            if(auto tail = expression->getTail())
            {
                generateExpression(tail);
                m_emitter.pop(Registers::getReturn());
            }
            
            auto offset = m_emitter.getStackPosition() - stack_position;
            if(offset)
            {
                auto bytes = static_cast<Types::i64>(8ul * offset);
                m_emitter.binary(bytes > 0l? Emitter::BinaryInstruction::Add: Emitter::BinaryInstruction::Subtract, Registers::getStack(), std::to_string(std::abs(bytes)));
            }
            
            m_emitter.push(Registers::getReturn());
            m_variables.endScope();
        }

        void generateExpression(const BoundExpression* expression)
        {
            if(auto literal = dynamic_cast<const BoundLiteralExpression*>(expression))
                generateLiteralExpression(literal);
            else if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(expression))
            {
                auto variable = m_variables.get(identifier->getValue());
                
                if(const auto static_identifier = std::get_if<std::string>(variable))
                    m_emitter.push(*static_identifier);
                else m_emitter.push(m_emitter.getStackOffset(std::get<std::size_t>(*variable)));
            }
            else if(auto if_expression = dynamic_cast<const BoundIfExpression*>(expression))
                generateIfExpression(if_expression);

            else if(auto while_expression = dynamic_cast<const BoundWhileExpression*>(expression))
                generateWhileExpression(while_expression);

            else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
                generateUnaryExpression(unary_expression);

            else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
                generateBinaryExpression(binary_expression);

            else if(auto block_expression = dynamic_cast<const BoundBlockExpression*>(expression))
                generateBlockExpression(block_expression);

            else if(auto external_call = dynamic_cast<const BoundExternalCallExpression*>(expression))
                generateExternalCallExpression(external_call);

            else if(auto function_call = dynamic_cast<const BoundFunctionCallExpression*>(expression))
                generateFunctionCallExpression(function_call);

            else if(auto conversion = dynamic_cast<const BoundConversionExpression*>(expression))
                generateConversionExpression(conversion);

            else if(auto type_expression = dynamic_cast<const BoundTypeExpression*>(expression))
                generateTypeExpression(type_expression);

            else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
                generateIndexExpression(index_expression);

            else throw LINC_EXCEPTION_ILLEGAL_STATE(expression);
        }

        void generateReturnStatement(const BoundReturnStatement* statement)
        {
            if(m_isMain)
            {
                m_emitter.external(s_systemExit);
                generateExpression(statement->getExpression());

                if(statement->getExpression()->getType().primitive == Types::Kind::_void)
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), std::to_string(0));
                else m_emitter.pop(Registers::getArgumentName(0));

                m_emitter.unary(Emitter::UnaryInstruction::Call, s_systemExit);
                return;
            }
            generateExpression(statement->getExpression());
            m_emitter.pop(Registers::getReturn());
            m_emitter.nullary(Emitter::NullaryInstruction::Leave);
            m_emitter.nullary(Emitter::NullaryInstruction::Return);
        }

        void generateIndexExpression(const BoundIndexExpression* expression)
        {
            if(expression->getType().kind != Types::type::Kind::Primitive)
                throw LINC_EXCEPTION("Indexing non-primitive expression not implemented");

            generateExpression(expression->getArray());
            generateExpression(expression->getIndex());
            m_emitter.pop(Registers::getPrimary());
            m_emitter.pop(Registers::getSecondary());
            m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::Byte),
                m_emitter.binaryAddress(Registers::getPrimary(), Registers::getSecondary(), Registers::Size::Byte));
            m_emitter.push(Registers::getPrimary());
        }

        void generateTypeExpression(const BoundTypeExpression* expression)
        {
            auto actual_type = expression->getActualType();
            auto literal = m_emitter.defineStringLiteral(actual_type.toString());
            m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), literal);
            m_emitter.push(Registers::getPrimary());
        }

        void generateConversionExpression(const BoundConversionExpression* expression)
        {
            auto initial_type = expression->getConversion()->getInitialType().primitive;
            auto return_type = expression->getConversion()->getReturnType().primitive;
            auto operand_size = getRegisterOperandSize(expression->getConversion()->getReturnType().primitive);

            generateExpression(expression->getExpression());
            m_emitter.pop(Registers::getPrimary());

            if(initial_type == Types::Kind::f32 && Types::isIntegral(return_type))
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::DoubleWord),
                    Emitter::InstructionKind::Float);
                m_emitter.binary(Emitter::BinaryInstruction::ConvertFloatToInt, Registers::getPrimary(operand_size), Registers::getPrimaryFloating());
            }
            else if(initial_type == Types::Kind::f64 && Types::isIntegral(return_type))
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(), Emitter::InstructionKind::Double);
                m_emitter.binary(Emitter::BinaryInstruction::ConvertDoubleToInt, Registers::getPrimary(operand_size), Registers::getPrimaryFloating());   
            }
            else if(Types::isIntegral(initial_type) && return_type == Types::Kind::f32)
            {
                m_emitter.binary(Emitter::BinaryInstruction::ConvertIntToFloat, Registers::getPrimaryFloating(), Registers::getPrimary(operand_size));
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::DoubleWord), Registers::getPrimaryFloating(),
                    Emitter::InstructionKind::Float);
            }
            else if(Types::isIntegral(initial_type) && return_type == Types::Kind::f64)
            {
                m_emitter.binary(Emitter::BinaryInstruction::ConvertIntToDouble, Registers::getPrimaryFloating(), Registers::getPrimary(operand_size));
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::QuadWord), Registers::getPrimaryFloating(),
                    Emitter::InstructionKind::Double);
            }
            else if(initial_type == Types::Kind::f32 && return_type == Types::Kind::f64)
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::DoubleWord),
                    Emitter::InstructionKind::Float);
                m_emitter.binary(Emitter::BinaryInstruction::ConvertFloatToDouble, Registers::getPrimaryFloating(), Registers::getPrimaryFloating());
            }
            else if(initial_type == Types::Kind::f64 && return_type == Types::Kind::f32)
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::QuadWord),
                    Emitter::InstructionKind::Double);
                m_emitter.binary(Emitter::BinaryInstruction::ConvertDoubleToFloat, Registers::getPrimaryFloating(), Registers::getPrimaryFloating());
            }

            m_emitter.push(Registers::getPrimary());
        }

        void generateFunctionCallExpression(const BoundFunctionCallExpression* expression)
        {
            const auto& name = expression->getName();

            for(std::size_t i{0ul}; i < expression->getArguments().size(); ++i)
            {
                generateExpression(expression->getArguments()[i].value.get());
                m_emitter.pop(Registers::getArgumentName(static_cast<std::uint8_t>(i)));
            }
            m_emitter.unary(Emitter::UnaryInstruction::Call, name);
            m_emitter.push(Registers::getReturn());
        }

        void generateExternalCallExpression(const BoundExternalCallExpression* expression)
        {
            const auto& name = expression->getName();
            
            for(std::size_t i{0ul}; i < expression->getArguments().size(); ++i)
            {
                generateExpression(expression->getArguments()[i].get());
                m_emitter.pop(Registers::getArgumentName(static_cast<std::uint8_t>(i)));
            }
            
            auto find = m_externalDefinitions.find(name);

            if(find == m_externalDefinitions.end())
            {
                m_emitter.external(name);
                m_externalDefinitions.insert(name);
            }
            m_emitter.unary(Emitter::UnaryInstruction::Call, name);
            m_emitter.push(Registers::getReturn());
        }

        void generateUnaryExpression(const BoundUnaryExpression* expression)
        {
            if(expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Typeof)
            {
                auto actual_type = expression->getOperand()->getType();
                auto literal = m_emitter.defineStringLiteral(actual_type.toString());
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), literal);
                m_emitter.push(Registers::getPrimary());
                return;
            }

            generateExpression(expression->getOperand());
            m_emitter.pop(Registers::getReturn());

            Registers::Size operand_size = getRegisterOperandSize(expression->getOperand()->getType().primitive);
            
            switch(expression->getOperator()->getKind())
            {
            case BoundUnaryOperator::Kind::UnaryPlus:
                if(expression->getOperand()->getType().primitive == Types::Kind::string)
                {
                    static constexpr auto string_length_function{"__string_length"};
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), Registers::getReturn());
                    m_emitter.unary(Emitter::UnaryInstruction::Call, string_length_function);
                    m_emitter.external(string_length_function);
                }
                m_emitter.push(Registers::getReturn());
                break;
            case BoundUnaryOperator::Kind::UnaryMinus:
                m_emitter.unary(Emitter::UnaryInstruction::Negate, Registers::getPrimary(operand_size));
                break;
            case BoundUnaryOperator::Kind::Increment:
                m_emitter.unary(Emitter::UnaryInstruction::Increment, Registers::getPrimary(operand_size));
                break;
            case BoundUnaryOperator::Kind::Decrement:
                m_emitter.unary(Emitter::UnaryInstruction::Decrement, Registers::getPrimary(operand_size));
                break;
            case BoundUnaryOperator::Kind::BitwiseNot:
            case BoundUnaryOperator::Kind::LogicalNot:
                m_emitter.unary(Emitter::UnaryInstruction::Not, Registers::getPrimary(operand_size));
                break;
            case BoundUnaryOperator::Kind::Stringify:
            {
                std::string to_string_symbol;
                switch(expression->getOperand()->getType().primitive)
                {
                case Types::Kind::type:
                case Types::Kind::string: m_emitter.push(Registers::getPrimary()); return;
                case Types::Kind::_void:
                {
                    static const auto void_literal = m_emitter.defineStringLiteral(LINC_GENERATORAMD64_STRING_LITERAL_VOID, "__literal_void");
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), void_literal);
                    return;
                }
                case Types::Kind::u8:
                case Types::Kind::u16:
                case Types::Kind::u32:
                case Types::Kind::u64:
                    to_string_symbol = "__unsigned_to_string";
                    break;
                case Types::Kind::i8:
                case Types::Kind::i16:
                case Types::Kind::i32:
                case Types::Kind::i64:
                    to_string_symbol = "__signed_to_string";
                    break;
                case Types::Kind::_char:
                case Types::Kind::_bool:
                    to_string_symbol = Logger::format("__$_to_string", Types::kindToString(expression->getOperand()->getType().primitive));
                    break;
                default:
                    throw LINC_EXCEPTION("Unimplemented type for to-string convertion");
                }
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), Registers::getPrimary());
                m_emitter.unary(Emitter::UnaryInstruction::Call, to_string_symbol);
                m_emitter.external(to_string_symbol);
                break;
            }
            default: throw LINC_EXCEPTION("Unimplemented unary expression type");
            }
            m_emitter.push(Registers::getPrimary());

            if(expression->getOperator()->getReturnType().isMutable)
            {
                auto name = dynamic_cast<const BoundIdentifierExpression*>(expression->getOperand())->getValue();
                auto variable = m_variables.get(name);
                std::string destination;

                if(auto static_identifier = std::get_if<std::string>(variable))
                    destination = *static_identifier;
                else destination = m_emitter.getStackOffset(std::get<std::size_t>(*variable)); 
                
                m_emitter.binary(Emitter::BinaryInstruction::Move, destination, Registers::getPrimary());
            }
        }

        void generateWhileExpression(const BoundWhileExpression* expression)
        {
            m_variables.beginScope();
            auto has_else = expression->hasElse();
            auto has_finally = expression->hasFinally();

            if(has_else || has_finally)
                m_emitter.binary(Emitter::BinaryInstruction::Xor, Registers::getConditional(), Registers::getConditional());

            auto test_label = m_emitter.label();
            auto exit_label = m_emitter.reserveLabel();
            auto jump_instruction = generateConditional(expression->getTestExpression());
            m_emitter.unary(jump_instruction, exit_label);

            generateExpression(expression->getWhileBody());
            m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getConditional(), std::to_string(-1));
            m_emitter.pop(Registers::getReturn());

            m_emitter.unary(Emitter::UnaryInstruction::Jump, test_label);
            m_emitter.label(exit_label);
            if(has_finally)
            {
                m_emitter.binary(Emitter::BinaryInstruction::Test, Registers::getConditional(), Registers::getConditional());
                auto label_finally = m_emitter.reserveLabel();
                auto label_else = m_emitter.reserveLabel();

                m_emitter.unary(Emitter::UnaryInstruction::JumpIfZero, label_else);
                generateExpression(expression->getFinallyBody());
                m_emitter.pop(Registers::getReturn());
                
                if(has_else)
                    m_emitter.unary(Emitter::UnaryInstruction::Jump, label_finally);
                
                m_emitter.label(label_else);

                if(has_else)
                {
                    generateExpression(expression->getElseBody());
                    m_emitter.pop(Registers::getReturn());
                    m_emitter.label(label_finally);
                }
            }
            else if(has_else)
            {
                auto label_exit = m_emitter.reserveLabel();
                m_emitter.binary(Emitter::BinaryInstruction::Test, Registers::getConditional(), Registers::getConditional());
                m_emitter.unary(Emitter::UnaryInstruction::JumpIfNotZero, label_exit);
                generateExpression(expression->getElseBody());
                m_emitter.pop(Registers::getReturn());
                m_emitter.label(label_exit);
            }
            
            m_variables.endScope();
        }

        void generateIfExpression(const BoundIfExpression* expression)
        {
            m_variables.beginScope();
            auto jump_instruction = generateConditional(expression->getTestExpression());

            auto label_true = m_emitter.reserveLabel();
            auto label_false = m_emitter.reserveLabel();

            m_emitter.unary(jump_instruction, label_false);
            generateExpression(expression->getIfBody());
            m_emitter.pop(Registers::getReturn());

            auto has_else = expression->hasElse();

            if(has_else)
                m_emitter.unary(Emitter::UnaryInstruction::Jump, label_true);

            m_emitter.label(label_false);

            if(has_else)
            {
                generateExpression(expression->getElseBody());
                m_emitter.pop(Registers::getReturn());
                m_emitter.label(label_true);
            }
            m_variables.endScope();
        }

        void generateLiteralExpression(const BoundLiteralExpression* expression)
        {
            switch(expression->getType().primitive)
            {
            case Types::Kind::string:
            {
                auto value = m_emitter.defineStringLiteral(expression->getValue().getString());
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getReturn(), value);
                m_emitter.push(Registers::getReturn());
                break;
            }
            case Types::Kind::i64:
            case Types::Kind::u64:
                if((expression->getValue().getU64() & 0xf000000000000000) == 0)
                    m_emitter.push(std::to_string(expression->getValue().getU64()));
                else
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::QuadWord), std::to_string(expression->getValue().getI64()));
                    m_emitter.push(Registers::getPrimary());
                }
                break;
            case Types::Kind::u8:
            case Types::Kind::i8:
                m_emitter.push(std::to_string(expression->getValue().getU8()));
                break;
            case Types::Kind::u16:
            case Types::Kind::i16:
                m_emitter.push(std::to_string(expression->getValue().getU16()));
                break;
            case Types::Kind::i32:
            case Types::Kind::u32:
                if((expression->getValue().getU32() & 0xf0000000) == 0)
                    m_emitter.push(std::to_string(expression->getValue().getU32()));
                else
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::DoubleWord), std::to_string(expression->getValue().getI32()));
                    m_emitter.push(Registers::getPrimary());
                }
                break;
            case Types::Kind::_bool:
                m_emitter.push(expression->getValue().getBool()? std::to_string(-1): std::to_string(0));
                break;
            case Types::Kind::_char:
                m_emitter.push(std::to_string(+expression->getValue().getChar()));
                break;
            case Types::Kind::f32:
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), std::to_string(std::bit_cast<Types::i32>(
                    expression->getValue().getF32())));
                m_emitter.push(Registers::getPrimary());
                break;
            }
            case Types::Kind::f64:
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), std::to_string(std::bit_cast<Types::i64>(
                    expression->getValue().getF64())));
                m_emitter.push(Registers::getPrimary());
                break;
            }
            default: throw LINC_EXCEPTION("Literal type not yet implemented.");
            }
        }

        void generateVariableDeclaration(const BoundVariableDeclaration* declaration)
        {
            const auto& default_value = declaration->getDefaultValue();
            const auto& name = declaration->getName();

            if(m_variables.getScopeSize() == 1ul)
            {
                auto size = getRegisterOperandSize(declaration->getActualType().primitive);
                if(auto literal = dynamic_cast<const BoundLiteralExpression*>(*default_value); literal && default_value)
                {
                    auto value = static_cast<const BoundLiteralExpression*>(*default_value)->getValue();
                    std::string label_name;

                    switch(default_value.value()->getType().primitive)
                    {
                    case Types::Kind::string: label_name = m_emitter.defineStringLiteral(value.getString(), declaration->getActualType().isMutable); break;
                    default: label_name = m_emitter.defineNumeral(value.getU64(), size); break;
                    }
                    m_variables.append(name, label_name);
                    return;
                }

                auto label_name = m_emitter.defineNumeral(0ul, std::move(size));
                m_variables.append(name, label_name);
                return;
            }

            if(default_value)
                generateExpression(*default_value);
            else m_emitter.push(std::to_string(0));

            m_variables.append(name, m_emitter.getStackPosition());
        }

        void generateFunctionDeclaration(const BoundFunctionDeclaration* declaration)
        {
            m_variables.beginScope();
            if(declaration->getName() == "main")
            {
                const static auto entry_point = "_start";
                m_emitter.global(entry_point);
                m_emitter.label(entry_point);

                m_isMain = true;
                generateExpression(declaration->getBody());
                m_emitter.pop(Registers::getArgumentName(0));
                m_isMain = false;

                if(declaration->getReturnType().primitive == Types::Kind::_void)
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), std::to_string(0));
                    
                m_emitter.unary(Emitter::UnaryInstruction::Call, s_systemExit);
                m_emitter.external(s_systemExit);
                m_hasMain = true;
                m_variables.endScope();
                return;
            }

            m_emitter.global(declaration->getName());
            m_emitter.label(declaration->getName());
            m_emitter.prologue();
            
            for(std::size_t i{0ul}; i < declaration->getArguments().size(); ++i)
            {
                m_emitter.push(Registers::getArgumentName(i));
                m_variables.append(declaration->getArguments()[i]->getName(), m_emitter.getStackPosition());
            }

            generateExpression(declaration->getBody());
            m_emitter.pop(Registers::getReturn());
            m_emitter.epilogue();
            m_variables.endScope();
        }

        void generateBinaryExpression(const BoundBinaryExpression* expression)
        {
            generateExpression(expression->getLeft());
            generateExpression(expression->getRight());

            if(expression->getOperator()->getReturnType().primitive == Types::Kind::string && expression->getOperator()->getKind() == BoundBinaryOperator::Kind::Addition)
            {
                auto left_is_char = expression->getOperator()->getLeftType().primitive == Types::Kind::_char;
                auto right_is_char = expression->getOperator()->getRightType().primitive == Types::Kind::_char;
                
                m_emitter.pop(Registers::getArgumentName(1));
                m_emitter.pop(Registers::getArgumentName(0));

                if(left_is_char && right_is_char)
                {
                    static constexpr auto char_concat_function{"__char_concat"}; 
                    m_emitter.unary(Emitter::UnaryInstruction::Call, char_concat_function);
                    m_emitter.external(char_concat_function);
                }
                else if(left_is_char)
                {
                    static constexpr auto char_string_concat_function{"__char_string_concat"};
                    m_emitter.unary(Emitter::UnaryInstruction::Call, char_string_concat_function);
                    m_emitter.external(char_string_concat_function);
                }
                else if(expression->getOperator()->getRightType().primitive == Types::Kind::_char)
                {
                    static constexpr auto string_char_concat_function{"__string_char_concat"};
                    m_emitter.unary(Emitter::UnaryInstruction::Call, string_char_concat_function);
                    m_emitter.external(string_char_concat_function);
                }
                else
                {
                    static constexpr auto string_concat_function{"__string_concat"};
                    m_emitter.unary(Emitter::UnaryInstruction::Call, string_concat_function);
                    m_emitter.external(string_concat_function);
                }
                m_emitter.push(Registers::getReturn());
                return;
            }
            else
            {
                m_emitter.pop(Registers::getSecondary());
                m_emitter.pop(Registers::getPrimary());
            }

            std::string instruction;
            auto is_primitive = expression->getOperator()->getReturnType().kind == Types::type::Kind::Primitive;
            Emitter::InstructionKind kind{Emitter::InstructionKind::General};

            if(is_primitive && expression->getLeft()->getType().primitive == Types::Kind::f32)
            {
                kind = Emitter::InstructionKind::Float;
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::DoubleWord), kind);
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getSecondaryFloating(), Registers::getSecondary(Registers::Size::DoubleWord), kind);
            }
            else if(is_primitive && expression->getLeft()->getType().primitive == Types::Kind::f64)
            {
                kind = Emitter::InstructionKind::Double;
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::QuadWord), kind);
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getSecondaryFloating(), Registers::getSecondary(Registers::Size::QuadWord), kind);
            }
            bool is_sse = kind != Emitter::InstructionKind::General, is_signed = Types::isSigned(expression->getLeft()->getType().primitive);

            Registers::Size operand_size = getRegisterOperandSize(expression->getLeft()->getType().primitive);
            
            switch(expression->getOperator()->getKind())
            {
            case BoundBinaryOperator::Kind::Addition:
            case BoundBinaryOperator::Kind::AdditionAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Add, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else m_emitter.binary(Emitter::BinaryInstruction::Add, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind);

                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Subtraction:
            case BoundBinaryOperator::Kind::SubtractionAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Subtract, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else m_emitter.binary(Emitter::BinaryInstruction::Subtract, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind);

                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Multiplication:
            case BoundBinaryOperator::Kind::MultiplicationAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Multiply, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else if(is_signed) m_emitter.unary(Emitter::UnaryInstruction::SignedMultiply, Registers::getSecondary(operand_size), kind);
                else m_emitter.unary(Emitter::UnaryInstruction::UnsignedMultiply, Registers::getSecondary(operand_size), kind);

                m_emitter.push(Registers::getPrimary());
                break;
                break;
            case BoundBinaryOperator::Kind::Division:
            case BoundBinaryOperator::Kind::DivisionAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Divide, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else if(is_signed)
                {
                    m_emitter.nullary(operand_size == Registers::Size::DoubleWord? Emitter::NullaryInstruction::ConvertDoubleQuad:
                        Emitter::NullaryInstruction::ConvertQuadOctal);
                    m_emitter.unary(Emitter::UnaryInstruction::SignedDivide, Registers::getSecondary(operand_size), kind);
                }
                else
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Xor, Registers::getRemainder(operand_size), Registers::getRemainder(operand_size), kind);
                    m_emitter.unary(Emitter::UnaryInstruction::UnsignedDivide, Registers::getSecondary(operand_size), kind);
                }
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Modulo:
            case BoundBinaryOperator::Kind::ModuloAssignment:
                switch(kind)
                {
                case Emitter::InstructionKind::General:
                    m_emitter.binary(Emitter::BinaryInstruction::Xor, Registers::getRemainder(operand_size), Registers::getRemainder(operand_size), kind);
                    m_emitter.unary(Emitter::UnaryInstruction::UnsignedDivide, Registers::getSecondary(operand_size), kind);
                    m_emitter.push(Registers::getRemainder());
                    break;
                case Emitter::InstructionKind::Float:
                {
                    static constexpr auto function_name{"__mod_f32"};
                    m_emitter.unary(Emitter::UnaryInstruction::Call, function_name);
                    m_emitter.push(Registers::getReturn());
                    m_emitter.external(function_name);
                    break;
                }
                case Emitter::InstructionKind::Double:
                {
                    static constexpr auto function_name{"__mod_f64"};
                    m_emitter.unary(Emitter::UnaryInstruction::Call, function_name);
                    m_emitter.push(Registers::getReturn());
                    m_emitter.external(function_name);
                    break;
                }
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
                
                break;
            case BoundBinaryOperator::Kind::NotEquals:
            case BoundBinaryOperator::Kind::BitwiseXor:
                m_emitter.binary(Emitter::BinaryInstruction::Xor, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Less:
                if(is_sse) m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                else m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind); 
                m_emitter.unary(Emitter::UnaryInstruction::SetIfLess, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Greater:
                if(is_sse) m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                else m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind); 
                m_emitter.unary(Emitter::UnaryInstruction::SetIfGreater, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::LessEqual:
                if(is_sse) m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                else m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind); 
                m_emitter.unary(Emitter::UnaryInstruction::SetIfLessEqual, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::GreaterEqual:
                if(is_sse) m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                else m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind); 
                m_emitter.unary(Emitter::UnaryInstruction::SetIfGreaterEqual, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Equals:
                if(expression->getLeft()->getType().primitive == Types::Kind::string)
                {
                    static constexpr auto string_equals_function{"__string_equals"};
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), Registers::getPrimary());
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(1), Registers::getSecondary());
                    m_emitter.unary(Emitter::UnaryInstruction::Call, string_equals_function);
                    m_emitter.external(string_equals_function);
                    m_emitter.push(Registers::getReturn());
                    break;
                }
                if(is_sse) m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                else m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind); 
                m_emitter.unary(Emitter::UnaryInstruction::SetIfEqual, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Assignment:
            {
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), Registers::getSecondary(), kind);
                m_emitter.push(Registers::getPrimary());
            }
            case BoundBinaryOperator::Kind::LogicalAnd:
            case BoundBinaryOperator::Kind::BitwiseAnd:
                m_emitter.binary(Emitter::BinaryInstruction::And, Registers::getPrimary(), Registers::getSecondary(), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::LogicalOr:
            case BoundBinaryOperator::Kind::BitwiseOr:
                m_emitter.binary(Emitter::BinaryInstruction::Or, Registers::getPrimary(), Registers::getSecondary(), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::BitwiseShiftLeft:
                if constexpr(Registers::getCount() != Registers::getSecondary())
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getCount(), Registers::getSecondary());
                m_emitter.binary(Emitter::BinaryInstruction::BitShiftLeft, Registers::getPrimary(), Registers::getCount(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::BitwiseShiftRight:
                if constexpr(Registers::getCount() != Registers::getSecondary())
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getCount(), Registers::getSecondary());
                m_emitter.binary(Emitter::BinaryInstruction::BitShiftRight, Registers::getPrimary(), Registers::getCount(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            default: throw LINC_EXCEPTION("Unimplemented binary expresssion kind.");
            }

            if(expression->getOperator()->getReturnType().isMutable)
            {
                auto name = dynamic_cast<const BoundIdentifierExpression*>(expression->getLeft())->getValue();
                auto variable = m_variables.get(name);
                std::string destination;
                if(auto static_identifier = std::get_if<std::string>(variable))
                    destination = *static_identifier;
                else destination = m_emitter.getStackOffset(std::get<std::size_t>(*variable)); 
                m_emitter.binary(Emitter::BinaryInstruction::Move, destination, Registers::getPrimary(), Emitter::InstructionKind::General);
            }
        }

        Emitter::UnaryInstruction generateConditional(const BoundExpression* expression)
        {
            if(auto binary = dynamic_cast<const BoundBinaryExpression*>(expression))
            {
                switch(binary->getOperator()->getKind())
                {
                case BoundBinaryOperator::Kind::Equals:
                    if(binary->getLeft()->getType().primitive == Types::Kind::string) break;
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfNotEqual);
                case BoundBinaryOperator::Kind::NotEquals:
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfEqual);
                case BoundBinaryOperator::Kind::Greater:
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfLessEqual);
                case BoundBinaryOperator::Kind::GreaterEqual:
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfLess);
                case BoundBinaryOperator::Kind::Less:
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfGreaterEqual);
                case BoundBinaryOperator::Kind::LessEqual:
                    return generateAndCompare(binary->getLeft(), binary->getRight(), Emitter::UnaryInstruction::JumpIfGreater);
                default: break;
                }
            }
            
            generateExpression(expression);
            m_emitter.pop(Registers::getPrimary());
            m_emitter.test(Registers::getPrimary());
            return Emitter::UnaryInstruction::JumpIfZero;
        }

        Emitter::UnaryInstruction generateAndCompare(const BoundExpression* left, const BoundExpression* right, Emitter::UnaryInstruction jump_instruction)
        {
            generateExpression(left);
            generateExpression(right);

            m_emitter.pop(Registers::getSecondary());
            m_emitter.pop(Registers::getPrimary());
            m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimary(), Registers::getSecondary());

            return jump_instruction;
        }

        static constexpr Registers::Size getRegisterOperandSize(Types::Kind primitive)
        {
            switch(primitive)
            {
            case Types::Kind::u8:
            case Types::Kind::i8:
            case Types::Kind::_bool:
            case Types::Kind::_char:
                return Registers::Size::Byte;
                break;
            case Types::Kind::u16:
            case Types::Kind::i16:
                return Registers::Size::Word;
                break;
            case Types::Kind::u32:
            case Types::Kind::i32:
            case Types::Kind::f32:
                return Registers::Size::DoubleWord;
                break;
            default:
                return Registers::Size::QuadWord;
            }
        }
    private:
        const BoundProgram* const m_program;
        const Target::Platform m_platform;
        Emitter m_emitter;
        std::unordered_set<std::string> m_externalDefinitions;
        ScopeStack<Variable> m_variables;
        bool m_hasMain{false}, m_isMain{false};
        constexpr static auto s_systemExit{"sys_exit"};
    };
}
