#pragma once
#include <linc/System.hpp>
#include <linc/Include.hpp>
#define LINC_EMITTER_LITERAL_INDENT "    "

namespace linc
{
    class EmitterAMD64 final
    {
    public:
        enum class UnaryInstruction : unsigned char
        {
            Push, Pop, Call, Jump, JumpIfZero, JumpIfNotZero, JumpIfEqual, JumpIfNotEqual, JumpIfGreater, JumpIfLess, JumpIfGreaterEqual, JumpIfLessEqual,
            Negate, Increment, Decrement, Not, UnsignedMultiply, SignedMultiply, UnsignedDivide, SignedDivide, SetIfEqual, SetIfNotEqual,
            SetIfGreater, SetIfGreaterEqual, SetIfLess, SetIfLessEqual
        };

        enum class NullaryInstruction: unsigned char
        {
            Syscall, Leave, Return, ConvertDoubleQuad, ConvertQuadOctal
        };

        enum class BinaryInstruction: unsigned char
        {
            Move, MoveExtend, And, Or, Xor, Add, Subtract, Multiply, Divide, Compare, Test, ConvertF32ToI32, ConvertF64ToI32
        };

        enum class InstructionKind: unsigned char
        {
            General, Float, Double
        };

        [[nodiscard]] inline const std::string& getDataSegment() const { return m_dataSegment; }
        [[nodiscard]] inline const std::string& getCodeSegment() const { return m_codeSegment; }
        [[nodiscard]] inline std::string get() const { return Logger::format("$segment .data\n$segment .text\n$", m_externalSymbols, m_dataSegment, m_codeSegment); }
        [[nodiscard]] inline std::size_t getStackPosition() const { return m_stackPosition; } 

        inline void reset() { m_dataSegment = m_codeSegment = std::string{}; }
        inline void emit(const std::string& line, bool has_indent = true)
        {
            Logger::append(m_codeSegment, "$:$\n", has_indent? s_indent: std::string{}, line);
        }

        inline void emitData(const std::string& line, bool has_indent = true)
        {
            Logger::append(m_dataSegment, "$:$\n", has_indent? s_indent: std::string{}, line);
        }

        std::string reserveLabel() { return Logger::format("L$", m_labelCounter++); }

        std::string label(std::string name = std::string{})
        {
            auto label_name = name.empty()? Logger::format("L$", m_labelCounter++): name;
            emit(label_name + ':', false);
            return label_name;
        }

        inline void push(std::string_view register_name) { unary(UnaryInstruction::Push, register_name); ++m_stackPosition; }
        inline void pop(std::string_view register_name) { unary(UnaryInstruction::Pop, register_name); --m_stackPosition; }
        inline void test(std::string_view register_name) { binary(BinaryInstruction::Test, register_name, register_name); }
        
        inline void external(std::string_view symbol_name) { Logger::append(m_externalSymbols, "extern $\n", symbol_name); }
        inline void global(std::string_view symbol_name) { Logger::append(m_externalSymbols, "global $\n", symbol_name); }

        inline void prologue() { push(Registers::getBase()); binary(BinaryInstruction::Move, Registers::getBase(), Registers::getStack()); }
        inline void epilogue() { nullary(NullaryInstruction::Leave); nullary(NullaryInstruction::Return); }

        std::string getStackOffset(std::size_t offset)
        {
            auto signed_offset = static_cast<Types::i64>(8ul * (getStackPosition() - offset));
            return "qword [rsp" + (signed_offset == 0l? std::string(1ul, ']'):
                signed_offset > 0l? Logger::format(" + $]", signed_offset): Logger::format(" - $]", -signed_offset));
        }

        std::string stringLiteral(std::string_view contents)
        {
            auto find = m_literalMap.find(contents);
            
            if(find != m_literalMap.end())
                return find->second;

            auto label_name = Logger::format("L$", m_labelCounter++);
            std::string literal{'"'};
            literal.reserve(contents.size() * 2ul);

            for(std::size_t i{0ul}; i < contents.size(); ++i)
            {
                char character = contents[i];
                char previous_character = i != 0ul? contents[i - 1ul]: ' ';
                char next_character = i != contents.size() - 1ul? contents[i + 1ul]: '\0';

                if(character == '"' || !std::isprint(character))
                {
                    if(previous_character != '"' && std::isprint(previous_character))
                        literal += "\", ";
                    
                    literal += getHexadecimal(+character) + ", ";

                    if(next_character != '"' && std::isprint(next_character))
                        literal.push_back('"');
                }
                else
                {
                    literal.push_back(character);
                    if(!next_character)
                        literal += "\", ";
                }
            }

            m_literalMap[contents] = label_name;
            Logger::append(m_dataSegment, "$:$ db $0\n", s_indent, label_name, literal);
            return label_name;
        }

        inline void binary(BinaryInstruction instruction, std::string_view destination, std::string_view source, InstructionKind kind = InstructionKind::General)
        {
            emit(Logger::format("$ $, $", binaryInstructionToString(instruction, kind), destination, source));
        }

        inline void unary(UnaryInstruction instruction, std::string_view operand, InstructionKind kind = InstructionKind::General)
        {
            emit(Logger::format("$ $", unaryInstructionToString(instruction, kind), operand));
        }

        inline void nullary(NullaryInstruction instruction)
        {
            emit(Logger::format("$", nullaryInstructionToString(instruction)));
        }
    private:
        static std::string unaryInstructionToString(UnaryInstruction instruction, InstructionKind kind)
        {
            switch(kind)
            {
            case InstructionKind::General:
                switch(instruction)
                {
                case UnaryInstruction::Push: return "push";
                case UnaryInstruction::Pop: return "pop";
                case UnaryInstruction::Call: return "call";
                case UnaryInstruction::Jump: return "jmp";
                case UnaryInstruction::JumpIfZero: return "jz";
                case UnaryInstruction::JumpIfNotZero: return "jnz";
                case UnaryInstruction::JumpIfEqual: return "je";
                case UnaryInstruction::JumpIfNotEqual: return "jne";
                case UnaryInstruction::JumpIfGreater: return "jg";
                case UnaryInstruction::JumpIfLess: return "jl";
                case UnaryInstruction::JumpIfGreaterEqual: return "jge";
                case UnaryInstruction::JumpIfLessEqual: return "jle";
                case UnaryInstruction::Negate: return "neg";
                case UnaryInstruction::Increment: return "inc";
                case UnaryInstruction::Decrement: return "dec";
                case UnaryInstruction::Not: return "not";
                case UnaryInstruction::UnsignedMultiply: return "mul";
                case UnaryInstruction::SignedMultiply: return "imul";
                case UnaryInstruction::UnsignedDivide: return "div";
                case UnaryInstruction::SignedDivide: return "idiv";
                case UnaryInstruction::SetIfEqual: return "sete";
                case UnaryInstruction::SetIfNotEqual: return "setne";
                case UnaryInstruction::SetIfGreater: return "setg";
                case UnaryInstruction::SetIfGreaterEqual: return "setge";
                case UnaryInstruction::SetIfLess: return "setl";
                case UnaryInstruction::SetIfLessEqual: return "setle";
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::UnaryInstruction);
                }
            case InstructionKind::Float:
            case InstructionKind::Double:
                switch(instruction)
                {
                case UnaryInstruction::JumpIfEqual: return "je";
                case UnaryInstruction::JumpIfNotEqual: return "jne";
                case UnaryInstruction::JumpIfGreater: return "ja";
                case UnaryInstruction::JumpIfLess: return "jb";
                case UnaryInstruction::JumpIfGreaterEqual: return "jae";
                case UnaryInstruction::JumpIfLessEqual: return "jbe";
                case UnaryInstruction::Negate: return "neg";
                case UnaryInstruction::SetIfEqual: return "sete";
                case UnaryInstruction::SetIfNotEqual: return "setne";
                case UnaryInstruction::SetIfGreater: return "seta";
                case UnaryInstruction::SetIfGreaterEqual: return "setae";
                case UnaryInstruction::SetIfLess: return "setb";
                case UnaryInstruction::SetIfLessEqual: return "setbe";
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::UnaryInstruction);
                }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::InstructionKind);
            }
        }

        static std::string binaryInstructionToString(BinaryInstruction instruction, InstructionKind kind)
        {
            switch(kind)
            {
            case InstructionKind::General:
                switch(instruction)
                {
                case BinaryInstruction::Move: return "mov";
                case BinaryInstruction::MoveExtend: return "movzx";
                case BinaryInstruction::And: return "and";
                case BinaryInstruction::Or: return "or";
                case BinaryInstruction::Xor: return "xor";
                case BinaryInstruction::Add: return "add";
                case BinaryInstruction::Subtract: return "sub";
                case BinaryInstruction::Compare: return "cmp";
                case BinaryInstruction::Test: return "test";
                case BinaryInstruction::ConvertF32ToI32: return "cvtss2si";
                case BinaryInstruction::ConvertF64ToI32: return "cvtsd2si";
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::UnaryInstruction);
                }
            case InstructionKind::Float:
                switch(instruction)
                {
                case BinaryInstruction::Move: return "movd";
                case BinaryInstruction::Add: return "addss";
                case BinaryInstruction::Subtract: return "subss";
                case BinaryInstruction::Multiply: return "mulss";
                case BinaryInstruction::Divide: return "divss";
                case BinaryInstruction::Compare: return "ucomiss";
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::BinaryInstruction);
                }
            case InstructionKind::Double:
                switch(instruction)
                {
                case BinaryInstruction::Move: return "movq";
                case BinaryInstruction::Add: return "addsd";
                case BinaryInstruction::Subtract: return "subsd";
                case BinaryInstruction::Multiply: return "mulsd";
                case BinaryInstruction::Divide: return "divsd";
                case BinaryInstruction::Compare: return "ucomisd";
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::BinaryInstruction);
                }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::InstructionKind);
            }
        }

        static std::string nullaryInstructionToString(NullaryInstruction instruction)
        {
            switch(instruction)
            {
            case NullaryInstruction::Syscall: return "syscall";
            case NullaryInstruction::Leave: return "leave";
            case NullaryInstruction::Return: return "ret";
            case NullaryInstruction::ConvertDoubleQuad: return "cdq";
            case NullaryInstruction::ConvertQuadOctal: return "cqo";
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(EmitterAMD64::NullaryInstruction);
            }
        }

        static std::string getHexadecimal(int value)
        {
            static char digits[] = "0123456789ABCDEF";
            std::string result{'0', 'x'};

            while(value != 0)
            {
                result += digits[value % 16];
                value /= 16;
            }

            return result;
        }

        std::string m_dataSegment, m_codeSegment, m_externalSymbols;
        std::size_t m_labelCounter{0ul}, m_stackPosition{0ul};
        std::unordered_map<std::string_view, std::string> m_literalMap;
        static constexpr const char* s_indent{LINC_EMITTER_LITERAL_INDENT};
    };
}
