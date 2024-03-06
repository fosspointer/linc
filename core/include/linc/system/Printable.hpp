#pragma once
#include <linc/system/Value.hpp>
#include <linc/Include.hpp>


namespace linc
{
    class Printable final
    {
    public:
        enum class Type
        {
            String, SignedIntegral, UnsignedIntegral, Floating, Nullptr, Boolean, Character, PrimitiveValue, ArrayValue, Value
        };

        Printable(const Value& value)
            :m_value(value), m_type(Type::Value)
        {}

        Printable(const PrimitiveValue& value)
            :m_primitiveValue(value), m_type(Type::PrimitiveValue)
        {}

        Printable(const ArrayValue& value)
            :m_arrayValue(value), m_type(Type::ArrayValue)
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
            case Type::Value: m_value = other.m_value; break;
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
            case Type::Value: m_value = other.m_value; break;
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
            case Type::Value: m_value = printable.m_value; break;
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
        
        std::string floatingToString(size_t precision) const
        {
            return precisionToString(m_floating, precision);
        }

        std::string signedToString() const
        {
            return std::to_string(m_signed);
        }

        std::string unsignedToString() const
        {
            return std::to_string(m_unsigned);
        }

        std::string nullptrToString() const
        {
            return "nullptr";
        }

        std::string booleanToString(bool lexical_bool) const
        {
            if(lexical_bool)
                return m_boolean? "true": "false";
            else
                return m_boolean? "1": "0";
        }

        std::string valueToString() const
        {
            return m_value.toString();
        }

        std::string primitiveValueToString() const
        {
            return m_primitiveValue.toString();
        }

        std::string arrayValueToString() const
        {
            return m_arrayValue.toString();
        }

        std::string characterToString() const
        {
            return std::string{m_character};
        }
    private:
        inline std::string precisionToString(std::float64_t value, size_t precision) const
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
            Value m_value;
            PrimitiveValue m_primitiveValue;
            ArrayValue m_arrayValue;
        };
        Type m_type;
    };
}