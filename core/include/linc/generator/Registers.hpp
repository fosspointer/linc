#pragma once
#include <linc/System.hpp>
#include <linc/Include.hpp>
#define LINC_REGISTERS_A 0
#define LINC_REGISTERS_B 1
#define LINC_REGISTERS_C 2
#define LINC_REGISTERS_D 3
#define LINC_REGISTERS_SOURCE 4
#define LINC_REGISTERS_DEST 5
#define LINC_REGISTERS_BASE 6
#define LINC_REGISTERS_STACK 7

namespace linc
{
    class Registers final
    {
    public:
        Registers() = delete;
        enum class Size
        {
            Byte, Word, DoubleWord, QuadWord
        };
        
        static auto& get()
        {
            static std::array<bool, s_registerCount> registers = {false};
            return registers;
        }

        static const std::string& getName(std::uint8_t index)
        {
            static const std::array<std::string, s_registerCount> string_map{
                "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
            };
            return string_map.at(index); 
        }

        static std::string getArgumentName(std::uint8_t index, Size size = Size::QuadWord)
        {
            const std::array<std::string, s_argumentCount> string_map{
                std::string{getRegister(LINC_REGISTERS_DEST, size)},
                std::string{getRegister(LINC_REGISTERS_SOURCE, size)},
                std::string{getRegister(LINC_REGISTERS_D, size)},
                std::string{getRegister(LINC_REGISTERS_C, size)},
                std::string{getRegister(8, size)},
                std::string{getRegister(9, size)}
            };
            return string_map.at(index); 
        }

        inline static constexpr std::string getReturn(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_A, size); }
        inline static constexpr std::string getStack(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_STACK, size); }
        inline static constexpr std::string getBase(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_BASE, size); }
        inline static constexpr std::string getPrimary(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_A, size); }
        inline static constexpr std::string getSecondary(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_C, size); }
        inline static constexpr std::string getCount(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_C, size); }
        inline static constexpr std::string getRemainder(Size size = Size::QuadWord) { return getRegister(LINC_REGISTERS_D, size); }
        inline static constexpr std::string getConditional(Size size = Size::QuadWord) { return getRegister(15, size); }
        inline static constexpr std::string getPrimaryFloating() { return "xmm0"; }
        inline static constexpr std::string getSecondaryFloating() { return "xmm1"; }

        inline static constexpr std::string getRegister(unsigned char order, Size size)
        {
            if(order >= 16)
                throw LINC_EXCEPTION_INVALID_INPUT("The specified register identifier was out of bounds (0-15)");

            if(order < 4) // registers a, b, c, d
            {
                const auto naming = std::string(1ul, 'a' + order);
                switch(size)
                {
                case Size::Byte: return naming + 'l';
                case Size::Word: return naming + 'x';
                case Size::DoubleWord: return 'e' + naming + 'x';
                case Size::QuadWord: return 'r' + naming + 'x';
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(size);
                }
            }
            else if(order < 8) // registers si, di, bp, sp
            {
                const static std::string naming_list[]{"si", "di", "bp", "sp"};
                const auto naming = naming_list[static_cast<std::size_t>(order) - 4ul];
                switch(size)
                {
                case Size::Byte: return naming + 'l';
                case Size::Word: return naming;
                case Size::DoubleWord: return 'e' + naming;
                case Size::QuadWord: return 'r' + naming;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(size);
                }
            }

            const auto naming = 'r' + std::to_string(order); // registers r8-r15
            switch(size)
            {
            case Size::Byte: return naming + 'b';
            case Size::Word: return naming + 'w';
            case Size::DoubleWord: return naming + 'd';
            case Size::QuadWord: return naming;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(size);
            }
        }

        static std::uint8_t allocate()
        {
            auto& registers = get();

            for(std::uint8_t i{0}; i < s_registerCount; ++i)
                if(!registers[i])
                {
                    registers[i] = true;
                    return i;
                }

            throw LINC_EXCEPTION_INVALID_INPUT("Ran out of available registers to allocate.");
        }

        static void free(std::uint8_t register_index)
        {
            auto& _register = get().at(register_index);
            
            if(_register == false)
                throw LINC_EXCEPTION_INVALID_INPUT("Tried to free a register, but it was unallocated.");

            _register = false;
        }
    private:
        static constexpr std::size_t s_registerCount{8ul}, s_argumentCount{6ul};
    };
}