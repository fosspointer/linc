#pragma once
#include <string>
#include <stdfloat>
#include <cstdint>
#include <sstream>
#include <linc/system/TypedValue.hpp>

namespace linc
{
    class Printable final
    {
    public:
        enum class Type
        {
            String, SignedIntegral, UnsignedIntegral, Floating, Nullptr, Boolean, Character, TypedValue
        };

        Printable(const TypedValue& value)
            :m_typedValue(value), m_type(Type::TypedValue)
        {}

        Printable(const std::string& str)
            :m_string(str.c_str()), m_type(Type::String)
        {}

        Printable(const char* str)
            :m_string(str), m_type(Type::String)
        {}

        Printable(uint8_t value)
            :m_unsigned(static_cast<std::uint64_t>(value)), m_type(Type::UnsignedIntegral)
        {}

        Printable(uint16_t value)
            :m_unsigned(static_cast<std::uint64_t>(value)), m_type(Type::UnsignedIntegral)
        {}

        Printable(uint32_t value)
            :m_unsigned(static_cast<std::uint64_t>(value)), m_type(Type::UnsignedIntegral)
        {}

        Printable(uint64_t value)
            :m_unsigned(value), m_type(Type::UnsignedIntegral)
        {}

        Printable(int8_t value)
            :m_signed(static_cast<std::int64_t>(value)), m_type(Type::SignedIntegral)
        {}

        Printable(int16_t value)
            :m_signed(static_cast<std::int64_t>(value)), m_type(Type::SignedIntegral)
        {}

        Printable(int32_t value)
            :m_signed(static_cast<std::int64_t>(value)), m_type(Type::SignedIntegral)
        {}

        Printable(int64_t value)
            :m_signed(value), m_type(Type::SignedIntegral)
        {}
        
        Printable(std::float32_t value)
            :m_floating(static_cast<std::float64_t>(value)), m_type(Type::Floating)
        {}

        Printable(std::float64_t value)
            :m_floating(value), m_type(Type::Floating)
        {}

        Printable(std::nullptr_t value)
            :m_nullptr(value), m_type(Type::Nullptr)
        {}

        Printable(bool value)
            :m_boolean(value), m_type(Type::Boolean)
        {}

        Printable(char value)
            :m_character(value), m_type(Type::Character)
        {}

        const Type& getType() const { return m_type; }

        const char* getString() const { return m_string; }
        
        std::string floatingToString(size_t precision)
        {
            return precToString(m_floating, precision);
        }

        std::string signedToString()
        {
            return std::to_string(m_signed);
        }

        std::string unsignedToString()
        {
            return std::to_string(m_unsigned);
        }

        std::string nullptrToString()
        {
            return "nullptr";
        }

        std::string booleanToString(bool lexical_bool)
        {
            if(lexical_bool)
                return m_boolean? "true": "false";
            else
                return m_boolean? "1": "0";
        }

        std::string typedValueToString(bool lexical_bool, size_t precision)
        {
            switch(m_typedValue.getType())
            {
            case Types::Type::u8:  return Printable(m_typedValue.getU8()).unsignedToString();
            case Types::Type::u16: return Printable(m_typedValue.getU16()).unsignedToString();
            case Types::Type::u32: return Printable(m_typedValue.getU32()).unsignedToString();
            case Types::Type::u64: return Printable(m_typedValue.getU64()).unsignedToString();
            case Types::Type::i8:  return Printable(m_typedValue.getI8()).signedToString();
            case Types::Type::i16: return Printable(m_typedValue.getI16()).signedToString();
            case Types::Type::i32: return Printable(m_typedValue.getI32()).signedToString();
            case Types::Type::i64: return Printable(m_typedValue.getI64()).signedToString();
            case Types::Type::_char: return Printable(m_typedValue.getChar()).characterToString();
            case Types::Type::_bool: return Printable(m_typedValue.getBool()).booleanToString(lexical_bool);
            case Types::Type::f32: return Printable(m_typedValue.getF32()).floatingToString(precision);
            case Types::Type::f64: return Printable(m_typedValue.getF64()).floatingToString(precision);
            case Types::Type::string: return m_typedValue.getString();
            case Types::Type::_void: return "{}";
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        std::string characterToString()
        {
            return std::string{m_character};
        }
    private:
        inline std::string precToString(std::float64_t value, size_t precision)
        {
            std::ostringstream out;
            out.precision(precision);
            out << std::fixed << value;
            return std::move(out).str();
        }

        union 
        {
            std::nullptr_t m_nullptr;
            std::float64_t m_floating;
            std::uint64_t m_unsigned;
            std::int64_t m_signed;
            const char* m_string;
            bool m_boolean;
            char m_character;
            TypedValue m_typedValue;
        };
        Type m_type;
    };
}