#pragma once
#include <linc/generator/Registers.hpp>
#include <linc/generator/Target.hpp>
#include <linc/generator/EmitterAMD64.hpp>

#define LINC_EXIT_PROGRAM_FAILURE 5
#define LINC_EXIT_PROGRAM_SUCCESS 0

namespace linc
{
    class GeneratorAMD64 final
    {
    public:
        using Emitter = EmitterAMD64;
        GeneratorAMD64(const BoundProgram* program, Target::Platform platform)
            :m_program(program), m_platform(platform)
        {}

        using Variable = std::size_t;

        std::pair<std::string, bool> generateProgram()
        {
            m_hasMain = {};
            m_emitter.reset();

            for(const auto& declaration: m_program->declarations)
                generateDeclaration(declaration.get());

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
                generateExpression(expression_statement->getExpression());

            else if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
                generateDeclaration(declaration_statement->getDeclaration());

            else throw LINC_EXCEPTION("Statement type not yet implemented.");
        }

        void generateExpression(const BoundExpression* expression)
        {
            if(auto literal = dynamic_cast<const BoundLiteralExpression*>(expression))
                generateLiteralExpression(literal);
            else if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(expression))
            {
                auto variable = m_variables.at(identifier->getValue());
                m_emitter.push(m_emitter.getStackOffset(variable));
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
            {
                auto stack_position = m_emitter.getStackPosition();
                ++m_scope;

                for(const auto& item: block_expression->getStatements())
                    generateStatement(item.get());
                
                --m_scope;
                auto offset = m_emitter.getStackPosition() - stack_position;
                if(offset)
                {
                    auto bytes = static_cast<Types::i64>(8ul * offset);
                    m_emitter.binary(bytes > 0l? Emitter::BinaryInstruction::Add: Emitter::BinaryInstruction::Subtract, Registers::getStack(), std::to_string(std::abs(bytes)));
                }
            }
            else if(auto external_call = dynamic_cast<const BoundExternalCallExpression*>(expression))
                generateExternalCallExpression(external_call);

            else if(auto function_call = dynamic_cast<const BoundFunctionCallExpression*>(expression))
                generateFunctionCallExpression(function_call);

            else if(auto conversion = dynamic_cast<const BoundConversionExpression*>(expression))
                generateConversionExpression(conversion);

            else throw LINC_EXCEPTION("Expression type not yet implemented.");
        }

        void generateConversionExpression(const BoundConversionExpression* expression)
        {
            auto initial_type = expression->getConversion()->getInitialType().primitive;
            auto return_type = expression->getConversion()->getReturnType().primitive;
            auto operand_size = getRegisterOperandSize(expression->getConversion()->getReturnType().primitive);

            generateExpression(expression->getExpression());
            if(initial_type == Types::Kind::f32 && Types::isIntegral(return_type))
                m_emitter.binary(Emitter::BinaryInstruction::ConvertF32ToI32, Registers::getPrimary(operand_size), Registers::getPrimaryFloating());

            else if(initial_type == Types::Kind::f64 && Types::isIntegral(return_type))
                m_emitter.binary(Emitter::BinaryInstruction::ConvertF64ToI32, Registers::getPrimary(operand_size), Registers::getPrimaryFloating());
        }

        void generateFunctionCallExpression(const BoundFunctionCallExpression* expression)
        {
            const auto& name = expression->getName();

            for(std::size_t i{0ul}; i < expression->getArguments().size(); ++i)
            {
                generateExpression(expression->getArguments()[i].value.get());
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
            generateExpression(expression->getOperand());
            m_emitter.pop(Registers::getReturn());

            Registers::Size operand_size = getRegisterOperandSize(expression->getOperand()->getType().primitive);

            switch(expression->getOperator()->getKind())
            {
            case BoundUnaryOperator::Kind::UnaryPlus:
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
                m_emitter.external("i32_to_string");
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0, operand_size), Registers::getPrimary(operand_size));
                m_emitter.unary(Emitter::UnaryInstruction::Call, "i32_to_string");
                break;
            default: throw LINC_EXCEPTION("Unimplemented unary expression type");
            }
            m_emitter.push(Registers::getPrimary());

            if(expression->getOperator()->getReturnType().isMutable)
            {
                auto name = dynamic_cast<const BoundIdentifierExpression*>(expression->getOperand())->getValue();
                auto offset = m_variables.at(name);
                m_emitter.binary(Emitter::BinaryInstruction::Move, m_emitter.getStackOffset(offset), Registers::getPrimary());
            }
        }

        void generateWhileExpression(const BoundWhileExpression* expression)
        {
            auto test_label = m_emitter.label();
            auto exit_label = m_emitter.reserveLabel();
            auto jump_instruction = generateConditional(expression->getTestExpression());
            m_emitter.unary(jump_instruction, exit_label);
            generateStatement(expression->getWhileBodyStatement());
            m_emitter.unary(Emitter::UnaryInstruction::Jump, test_label);
            m_emitter.label(exit_label);
        }

        void generateIfExpression(const BoundIfExpression* expression)
        {
            auto jump_instruction = generateConditional(expression->getTestExpression());

            auto label_true = m_emitter.reserveLabel();
            auto label_false = m_emitter.reserveLabel();

            m_emitter.unary(jump_instruction, label_false);
            generateStatement(expression->getIfBodyStatement());

            auto has_else = expression->getElseBodyStatement().has_value();

            if(has_else)
                m_emitter.unary(Emitter::UnaryInstruction::Jump, label_true);

            m_emitter.label(label_false);

            if(has_else)
            {
                generateStatement(expression->getElseBodyStatement().value());
                m_emitter.label(label_true);
            }
        }

        void generateLiteralExpression(const BoundLiteralExpression* expression)
        {
            switch(expression->getType().primitive)
            {
            case Types::Kind::string:
            {
                auto value = m_emitter.stringLiteral(expression->getValue().getString());
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getReturn(), value);
                m_emitter.push(Registers::getReturn());
                break;
            }
            case Types::Kind::i64:
            case Types::Kind::u64:
                if((expression->getValue().getU64() & 0xffffffff00000000) == 0)
                    m_emitter.push(std::to_string(expression->getValue().getI32()));
                else
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(Registers::Size::QuadWord), std::to_string(expression->getValue().getI64()));
                    m_emitter.push(Registers::getPrimary());
                }
                break;
            case Types::Kind::u8:
            case Types::Kind::i8:
                m_emitter.push(std::to_string(expression->getValue().getI8()));
                break;
            case Types::Kind::u16:
            case Types::Kind::i16:
                m_emitter.push(std::to_string(expression->getValue().getI16()));
                break;
            case Types::Kind::i32:
            case Types::Kind::u32:
                m_emitter.push(std::to_string(expression->getValue().getI32()));
                break;
            case Types::Kind::_bool:
                m_emitter.push(expression->getValue().getBool()? std::to_string(-1): std::to_string(0));
                break;
            case Types::Kind::_char:
                m_emitter.push(std::to_string(+expression->getValue().getChar()));
                break;
            case Types::Kind::f32:
            {
                Types::f32 value = expression->getValue().getF32();
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), std::to_string(reinterpret_cast<Types::i32&>(value)));
                m_emitter.push(Registers::getPrimary());
                break;
            }
            case Types::Kind::f64:
            {
                Types::f32 value = expression->getValue().getF32();
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), std::to_string(reinterpret_cast<Types::i32&>(value)));
                m_emitter.push(Registers::getPrimary());
                break;
            }
            default: throw LINC_EXCEPTION("Literal type not yet implemented.");
            }
        }

        void generateVariableDeclaration(const BoundVariableDeclaration* declaration)
        {
            const auto& default_value = declaration->getDefaultValue();
            if(default_value)
                generateExpression(*default_value);
            else m_emitter.push(std::to_string(0));

            m_variables.insert(std::pair<std::string, Variable>(declaration->getName(), m_emitter.getStackPosition()));
        }

        void generateFunctionDeclaration(const BoundFunctionDeclaration* declaration)
        {
            if(declaration->getName() == "main")
            {
                const static auto entry_point = "_start";
                m_emitter.global(entry_point);
                m_emitter.label(entry_point);

                generateStatement(declaration->getBody());

                if(declaration->getReturnType().primitive == Types::Kind::_void)
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getArgumentName(0), std::to_string(0));
                else m_emitter.pop(Registers::getArgumentName(0));
                
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(), std::to_string(60));
                m_emitter.nullary(Emitter::NullaryInstruction::Syscall);
                m_hasMain = true;
                return;
            }

            m_emitter.global(declaration->getName());
            m_emitter.label(declaration->getName());
            m_emitter.prologue();
            
            for(std::size_t i{0ul}; i < declaration->getArguments().size(); ++i)
            {
                m_emitter.push(Registers::getArgumentName(i));
                m_variables.insert(std::pair<std::string, Variable>(declaration->getArguments()[i]->getName(), m_emitter.getStackPosition()));
            }

            generateStatement(declaration->getBody());

            m_emitter.epilogue();
        }

        void generateBinaryExpression(const BoundBinaryExpression* expression)
        {
            generateExpression(expression->getLeft());
            generateExpression(expression->getRight());
            
            m_emitter.pop(Registers::getSecondary());
            m_emitter.pop(Registers::getPrimary());

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
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimaryFloating(), Registers::getPrimary(Registers::Size::DoubleWord), kind);
                m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getSecondaryFloating(), Registers::getSecondary(Registers::Size::DoubleWord), kind);
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
                else m_emitter.binary(Emitter::BinaryInstruction::Add, Registers::getPrimary(), Registers::getSecondary(), kind);

                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Subtraction:
            case BoundBinaryOperator::Kind::SubtractionAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Subtract, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else m_emitter.binary(Emitter::BinaryInstruction::Subtract, Registers::getPrimary(), Registers::getSecondary(), kind);

                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Multiplication:
            case BoundBinaryOperator::Kind::MultiplicationAssignment:
                if(is_sse)
                {
                    m_emitter.binary(Emitter::BinaryInstruction::Multiply, Registers::getPrimaryFloating(), Registers::getSecondaryFloating(), kind);
                    m_emitter.binary(Emitter::BinaryInstruction::Move, Registers::getPrimary(operand_size), Registers::getPrimaryFloating(), kind);
                }
                else if(is_signed) m_emitter.unary(Emitter::UnaryInstruction::SignedMultiply, Registers::getSecondary(), kind);
                else m_emitter.unary(Emitter::UnaryInstruction::UnsignedMultiply, Registers::getSecondary(), kind);

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
                if(is_sse) throw LINC_EXCEPTION_INVALID_INPUT("Floating point modulo operations are currently not supported.");
                m_emitter.unary(Emitter::UnaryInstruction::UnsignedDivide, Registers::getSecondary(operand_size), kind);
                m_emitter.push(Registers::getRemainder());
                break;
            case BoundBinaryOperator::Kind::NotEquals:
            case BoundBinaryOperator::Kind::BitwiseXor:
                m_emitter.binary(Emitter::BinaryInstruction::Xor, Registers::getPrimary(operand_size), Registers::getSecondary(operand_size), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Less:
                m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                m_emitter.unary(Emitter::UnaryInstruction::SetIfLess, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Greater:
                m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                m_emitter.unary(Emitter::UnaryInstruction::SetIfGreater, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::LessEqual:
                m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                m_emitter.unary(Emitter::UnaryInstruction::SetIfLessEqual, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::GreaterEqual:
                m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
                m_emitter.unary(Emitter::UnaryInstruction::SetIfGreaterEqual, Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.binary(Emitter::BinaryInstruction::MoveExtend, Registers::getPrimary(), Registers::getPrimary(Registers::Size::Byte), kind);
                m_emitter.push(Registers::getPrimary());
                break;
            case BoundBinaryOperator::Kind::Equals:
                m_emitter.binary(Emitter::BinaryInstruction::Compare, Registers::getPrimaryFloating(), Registers::getPrimaryFloating(), kind);
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
            default: throw LINC_EXCEPTION("Unimplemented binary expresssion kind.");
            }

            if(expression->getOperator()->getReturnType().isMutable)
            {
                auto name = dynamic_cast<const BoundIdentifierExpression*>(expression->getLeft())->getValue();
                auto offset = m_variables.at(name);
                m_emitter.binary(Emitter::BinaryInstruction::Move, m_emitter.getStackOffset(offset), Registers::getPrimary(), Emitter::InstructionKind::General);
            }
        }

        Emitter::UnaryInstruction generateConditional(const BoundExpression* expression)
        {
            if(auto binary = dynamic_cast<const BoundBinaryExpression*>(expression))
            {
                switch(binary->getOperator()->getKind())
                {
                case BoundBinaryOperator::Kind::Equals:
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
        std::unordered_map<std::string, Variable> m_variables;
        bool m_hasMain{false};
        std::size_t m_scope{0ul};
    };
}
