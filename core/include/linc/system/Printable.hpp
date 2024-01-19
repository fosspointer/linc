#pragma once
#include <linc/system/TypedValue.hpp>
#include <linc/Include.hpp>


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
            :m_type(Type::String)
        {
            new (&m_string) std::string{str};
        }

        Printable(const char* str)
            :m_type(Type::String)
        {
            new (&m_string) std::string{str};
        }

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

        Printable(const Printable& other)
            :m_type(other.m_type)
        {
            switch(m_type)
            {
            case Type::String: new (&m_string) std::string{other.m_string}; break;
            case Type::SignedIntegral: m_signed = other.m_signed; break;
            case Type::UnsignedIntegral: m_unsigned = other.m_unsigned; break;
            case Type::Floating: m_floating = other.m_floating; break;
            case Type::Nullptr: m_nullptr = other.m_nullptr; break;
            case Type::Boolean: m_boolean = other.m_boolean; break;
            case Type::Character: m_character = other.m_character; break;
            case Type::TypedValue: m_typedValue = other.m_typedValue; break;
            }
        }

        Printable(Printable&& other)
            :m_type(other.m_type)
        {
            other.m_type = Type::Nullptr;
            
            switch(m_type)
            {
            case Type::String: new (&m_string) std::string{std::move(other.m_string)}; other.m_nullptr = nullptr; break;
            case Type::SignedIntegral: m_signed = other.m_signed; break;
            case Type::UnsignedIntegral: m_unsigned = other.m_unsigned; break;
            case Type::Floating: m_floating = other.m_floating; break;
            case Type::Nullptr: m_nullptr = other.m_nullptr; break;
            case Type::Boolean: m_boolean = other.m_boolean; break;
            case Type::Character: m_character = other.m_character; break;
            case Type::TypedValue: m_typedValue = other.m_typedValue; break;
            }
        }

        Printable operator=(const Printable& printable)
        {
            m_type = printable.m_type;
            
            switch(m_type)
            {
            case Type::String: new (&m_string) std::string{printable.m_string}; break;
            case Type::SignedIntegral: m_signed = printable.m_signed; break;
            case Type::UnsignedIntegral: m_unsigned = printable.m_unsigned; break;
            case Type::Floating: m_floating = printable.m_floating; break;
            case Type::Nullptr: m_nullptr = printable.m_nullptr; break;
            case Type::Boolean: m_boolean = printable.m_boolean; break;
            case Type::Character: m_character = printable.m_character; break;
            case Type::TypedValue: m_typedValue = printable.m_typedValue; break;
            }

            return this;
        }

        ~Printable()
        {
            if(m_type == Type::String)
                m_string.~basic_string();
        }

        const Type& getType() const { return m_type; }

        const std::string& getString() const { return m_string; }
        
        std::string floatingToString(size_t precision)
        {
            return precisionToString(m_floating, precision);
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

        std::string typedValueToString()
        {
            return m_typedValue.toString();
        }

        std::string characterToString()
        {
            return std::string{m_character};
        }
    private:
        inline std::string precisionToString(std::float64_t value, size_t precision)
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
            std::string m_string;
            bool m_boolean;
            char m_character;
            TypedValue m_typedValue;
        };
        Type m_type;
    };
}