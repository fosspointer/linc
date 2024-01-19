#pragma once
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

#define LINC_TYPED_VALUE_LITERAL_TRUE "true"
#define LINC_TYPED_VALUE_LITERAL_FALSE "false"

#define LINC_TYPED_VALUE_CONSTRUCTOR(type) \
    TypedValue(Types::type value) \
        :m_value_##type(value), m_type(Types::Type::type) \
        {}

#define LINC_TYPED_VALUE_GETTERS(type, type_fn_name) \
    std::optional<Types::type> getIf##type_fn_name() const \
    { \
        if(m_type == Types::Type::type) \
            return m_value_##type; \
        else return std::nullopt; \
    } \
    Types::type get##type_fn_name() const \
    { \
        return m_value_##type; \
    }

#define LINC_TYPED_VALUE_OPERATOR_STRING(op, return_type) \
    return_type operator op(const TypedValue& _other) const \
    { \
        if(m_type == Types::Type::string || _other.m_type == Types::Type::string \
        || (m_type == Types::Type::_char && _other.m_type == Types::Type::_char)) \
            return toString() op _other.toString(); \
        \
        auto other = _other.convert(m_type); \
        switch(m_type) \
        { \
        case Types::Type::u8: return m_value_u8 op other.m_value_u8; \
        case Types::Type::u16: return m_value_u16 op other.m_value_u16; \
        case Types::Type::u32: return m_value_u32 op other.m_value_u32; \
        case Types::Type::u64: return m_value_u64 op other.m_value_u64; \
        case Types::Type::i8: return m_value_i8 op other.m_value_i8; \
        case Types::Type::i16: return m_value_i16 op other.m_value_i16; \
        case Types::Type::i32: return m_value_i32 op other.m_value_i32; \
        case Types::Type::i64: return m_value_i64 op other.m_value_i64; \
        case Types::Type::f32: return m_value_f32 op other.m_value_f32; \
        case Types::Type::f64: return m_value_f64 op other.m_value_f64; \
        case Types::Type::_char: return m_value__char op other.m_value__char; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type); \
        } \
    }

#define LINC_TYPED_VALUE_OPERATOR_NOSTRING(op, return_type) \
    return_type operator op(const TypedValue& _other) const \
    { \
        auto other = _other.convert(m_type); \
        switch(m_type) \
        { \
        case Types::Type::u8: return m_value_u8 op other.m_value_u8; \
        case Types::Type::u16: return m_value_u16 op other.m_value_u16; \
        case Types::Type::u32: return m_value_u32 op other.m_value_u32; \
        case Types::Type::u64: return m_value_u64 op other.m_value_u64; \
        case Types::Type::i8: return m_value_i8 op other.m_value_i8; \
        case Types::Type::i16: return m_value_i16 op other.m_value_i16; \
        case Types::Type::i32: return m_value_i32 op other.m_value_i32; \
        case Types::Type::i64: return m_value_i64 op other.m_value_i64; \
        case Types::Type::f32: return m_value_f32 op other.m_value_f32; \
        case Types::Type::f64: return m_value_f64 op other.m_value_f64; \
        case Types::Type::_char: return m_value__char op other.m_value__char; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type); \
        } \
    }

#define LINC_TYPED_VALUE_OPERATOR_UNARY_NUMERIC(op, return_type) \
return_type operator op() const \
    { \
        switch(m_type) \
        { \
        case Types::Type::u8: return op m_value_u8; \
        case Types::Type::u16: return op m_value_u16; \
        case Types::Type::u32: return op m_value_u32; \
        case Types::Type::u64: return op m_value_u64; \
        case Types::Type::i8: return op m_value_i8; \
        case Types::Type::i16: return op m_value_i16; \
        case Types::Type::i32: return op m_value_i32; \
        case Types::Type::i64: return op m_value_i64; \
        case Types::Type::f32: return op m_value_f32; \
        case Types::Type::f64: return op m_value_f64; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type); \
        } \
    }

namespace linc
{
    class TypedValue final
    {
    public:
        LINC_TYPED_VALUE_CONSTRUCTOR(u8)
        LINC_TYPED_VALUE_CONSTRUCTOR(u16)
        LINC_TYPED_VALUE_CONSTRUCTOR(u32)
        LINC_TYPED_VALUE_CONSTRUCTOR(u64)
        LINC_TYPED_VALUE_CONSTRUCTOR(i8)
        LINC_TYPED_VALUE_CONSTRUCTOR(i16)
        LINC_TYPED_VALUE_CONSTRUCTOR(i32)
        LINC_TYPED_VALUE_CONSTRUCTOR(i64)
        LINC_TYPED_VALUE_CONSTRUCTOR(f32)
        LINC_TYPED_VALUE_CONSTRUCTOR(f64)
        LINC_TYPED_VALUE_CONSTRUCTOR(_char)
        LINC_TYPED_VALUE_CONSTRUCTOR(_bool)
        
        TypedValue(const Types::string& value)
            :m_type(Types::Type::string)
        {
            new (&m_value_string) std::string{value};
        }

        ~TypedValue()
        {
            if(m_type == Types::Type::string)
                m_value_string.~basic_string();
        }

        TypedValue(TypedValue&& other)
            :m_type(other.m_type)
        {
            other.m_type = Types::Type::invalid; 

            switch (m_type)
            {
            case Types::Type::u8: m_value_u8 = other.m_value_u8; break;
            case Types::Type::u16: m_value_u16 = other.m_value_u16; break;
            case Types::Type::u32: m_value_u32 = other.m_value_u32; break;
            case Types::Type::u64: m_value_u64 = other.m_value_u64; break;
            case Types::Type::i8: m_value_i8 = other.m_value_i8; break;
            case Types::Type::i16: m_value_i16 = other.m_value_i16; break;
            case Types::Type::i32: m_value_i32 = other.m_value_i32; break;
            case Types::Type::i64: m_value_i64 = other.m_value_i64; break;
            case Types::Type::f32: m_value_f32 = other.m_value_f32; break;
            case Types::Type::f64: m_value_f64 = other.m_value_f64; break;
            case Types::Type::_char: m_value__char = other.m_value__char; break;
            case Types::Type::_void: m_value__void = other.m_value__void; break;
            case Types::Type::_bool: m_value__bool = other.m_value__bool; break;
            case Types::Type::string: new (&m_value_string) std::string{std::move(other.m_value_string)}; break;
            default: m_value_invalid = m_value_invalid; break;
            }
        }

        TypedValue(const TypedValue& other)
            :m_type(other.m_type)
        {
            switch (m_type)
            {
            case Types::Type::u8: m_value_u8 = other.m_value_u8; break;
            case Types::Type::u16: m_value_u16 = other.m_value_u16; break;
            case Types::Type::u32: m_value_u32 = other.m_value_u32; break;
            case Types::Type::u64: m_value_u64 = other.m_value_u64; break;
            case Types::Type::i8: m_value_i8 = other.m_value_i8; break;
            case Types::Type::i16: m_value_i16 = other.m_value_i16; break;
            case Types::Type::i32: m_value_i32 = other.m_value_i32; break;
            case Types::Type::i64: m_value_i64 = other.m_value_i64; break;
            case Types::Type::f32: m_value_f32 = other.m_value_f32; break;
            case Types::Type::f64: m_value_f64 = other.m_value_f64; break;
            case Types::Type::_char: m_value__char = other.m_value__char; break;
            case Types::Type::_void: m_value__void = other.m_value__void; break;
            case Types::Type::_bool: m_value__bool = other.m_value__bool; break;
            case Types::Type::string: new (&m_value_string) std::string{other.m_value_string}; break;
            default: m_value_invalid = m_value_invalid; break;
            }
        }

        TypedValue& operator=(const TypedValue& other)
        {
            m_type = other.m_type;
            switch (m_type)
            {
            case Types::Type::u8: m_value_u8 = other.m_value_u8; break;
            case Types::Type::u16: m_value_u16 = other.m_value_u16; break;
            case Types::Type::u32: m_value_u32 = other.m_value_u32; break;
            case Types::Type::u64: m_value_u64 = other.m_value_u64; break;
            case Types::Type::i8: m_value_i8 = other.m_value_i8; break;
            case Types::Type::i16: m_value_i16 = other.m_value_i16; break;
            case Types::Type::i32: m_value_i32 = other.m_value_i32; break;
            case Types::Type::i64: m_value_i64 = other.m_value_i64; break;
            case Types::Type::f32: m_value_f32 = other.m_value_f32; break;
            case Types::Type::f64: m_value_f64 = other.m_value_f64; break;
            case Types::Type::_char: m_value__char = other.m_value__char; break;
            case Types::Type::_void: m_value__void = other.m_value__void; break;
            case Types::Type::_bool: m_value__bool = other.m_value__bool; break;
            case Types::Type::string: new (&m_value_string) std::string{other.m_value_string}; break;
            default: m_value_invalid = m_value_invalid; break;
            }
            return *this;
        }

        bool isZero()
        {
            switch (m_type)
            {
            case Types::Type::u8: return m_value_u8 == 0ul;
            case Types::Type::u16: return m_value_u16 == 0ul;
            case Types::Type::u32: return m_value_u32 == 0ul;
            case Types::Type::u64: return m_value_u64 == 0ull;
            case Types::Type::i8: return m_value_i8 == 0l;
            case Types::Type::i16: return m_value_i16 == 0l;
            case Types::Type::i32: return m_value_i32 == 0l;
            case Types::Type::i64: return m_value_i64 == 0ll;
            case Types::Type::f32: return m_value_f32 == 0.0f;
            case Types::Type::f64: return m_value_f64 == 0.0l;
            default: return false;
            }
        }

        TypedValue operator++()
        {
            switch (m_type)
            {
            case Types::Type::u8: return ++m_value_u8;
            case Types::Type::u16: return ++m_value_u16;
            case Types::Type::u32: return ++m_value_u32;
            case Types::Type::u64: return ++m_value_u64;
            case Types::Type::i8: return ++m_value_i8;
            case Types::Type::i16: return ++m_value_i16;
            case Types::Type::i32: return ++m_value_i32;
            case Types::Type::i64: return ++m_value_i64;
            case Types::Type::f32: return ++m_value_f32;
            case Types::Type::f64: return ++m_value_f64;
            default: throw LINC_EXCEPTION_INVALID_INPUT("Cannot increment non numeric value");
            }
        }

        TypedValue operator--()
        {
            switch (m_type)
            {
            case Types::Type::u8: return --m_value_u8;
            case Types::Type::u16: return --m_value_u16;
            case Types::Type::u32: return --m_value_u32;
            case Types::Type::u64: return --m_value_u64;
            case Types::Type::i8: return --m_value_i8;
            case Types::Type::i16: return --m_value_i16;
            case Types::Type::i32: return --m_value_i32;
            case Types::Type::i64: return --m_value_i64;
            case Types::Type::f32: return --m_value_f32;
            case Types::Type::f64: return --m_value_f64;
            default: throw LINC_EXCEPTION_INVALID_INPUT("Cannot decrement non numeric value");
            }
        }
        
        LINC_TYPED_VALUE_GETTERS(u8, U8)
        LINC_TYPED_VALUE_GETTERS(u16, U16)
        LINC_TYPED_VALUE_GETTERS(u32, U32)
        LINC_TYPED_VALUE_GETTERS(u64, U64)
        LINC_TYPED_VALUE_GETTERS(i8, I8)
        LINC_TYPED_VALUE_GETTERS(i16, I16)
        LINC_TYPED_VALUE_GETTERS(i32, I32)
        LINC_TYPED_VALUE_GETTERS(i64, I64)
        LINC_TYPED_VALUE_GETTERS(f32, F32)
        LINC_TYPED_VALUE_GETTERS(f64, F64)
        LINC_TYPED_VALUE_GETTERS(_char, Char)
        LINC_TYPED_VALUE_GETTERS(_bool, Bool)
        LINC_TYPED_VALUE_GETTERS(string, String)

        // TypedValue(const char* str)
        //     :m_type(Types::Type::string)
        // {
        //     new (&m_value_string) std::string{str};
        // }

        LINC_TYPED_VALUE_OPERATOR_STRING(==, bool)
        LINC_TYPED_VALUE_OPERATOR_STRING(!=, bool)
        LINC_TYPED_VALUE_OPERATOR_STRING(+, TypedValue)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(-, TypedValue)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(*, TypedValue)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(/, TypedValue)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(>, bool)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(<, bool)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(>=, bool)
        LINC_TYPED_VALUE_OPERATOR_NOSTRING(<=, bool)

        Types::Variant toVariant() const
        {
            switch(m_type)
            {
            case Types::Type::u8: return m_value_u8;
            case Types::Type::u16: return m_value_u16;
            case Types::Type::u32: return m_value_u32;
            case Types::Type::u64: return m_value_u64;
            case Types::Type::i8: return m_value_i8;
            case Types::Type::i16: return m_value_i16;
            case Types::Type::i32: return m_value_i32;
            case Types::Type::i64: return m_value_i64;
            case Types::Type::f32: return m_value_f32;
            case Types::Type::f64: return m_value_f64;
            case Types::Type::_char: return m_value__char;
            case Types::Type::_void: return m_value__void;
            case Types::Type::string: return m_value_string;
            case Types::Type::invalid: return m_value_invalid;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        template <typename T>
        TypedValue convert() const
        {
            switch(m_type)
            {
            case Types::Type::u8: return static_cast<T>(m_value_u8);
            case Types::Type::u16: return static_cast<T>(m_value_u16);
            case Types::Type::u32: return static_cast<T>(m_value_u32);
            case Types::Type::u64: return static_cast<T>(m_value_u64);
            case Types::Type::i8: return static_cast<T>(m_value_i8);
            case Types::Type::i16: return static_cast<T>(m_value_i16);
            case Types::Type::i32: return static_cast<T>(m_value_i32);
            case Types::Type::i64: return static_cast<T>(m_value_i64);
            case Types::Type::f32: return static_cast<T>(m_value_f32);
            case Types::Type::f64: return static_cast<T>(m_value_f64);
            case Types::Type::_char: return static_cast<T>(m_value__char);
            case Types::Type::_bool: return static_cast<T>(m_value__bool);
            case Types::Type::_void:
            case Types::Type::string: return static_cast<T>(0);
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        TypedValue convert(Types::Type type) const
        {
            switch(type)
            {
            case Types::Type::u8: return convert<Types::u8>();
            case Types::Type::u16: return convert<Types::u16>();
            case Types::Type::u32: return convert<Types::u32>();
            case Types::Type::u64: return convert<Types::u64>();
            case Types::Type::i8: return convert<Types::i8>();
            case Types::Type::i16: return convert<Types::i16>();
            case Types::Type::i32: return convert<Types::i32>();
            case Types::Type::i64: return convert<Types::i64>();
            case Types::Type::f32: return convert<Types::f32>();
            case Types::Type::f64: return convert<Types::f64>();
            case Types::Type::_char: return convert<Types::_char>();
            case Types::Type::_bool: return convert<Types::_bool>();
            case Types::Type::string: return toString();
            case Types::Type::_void: return TypedValue::voidValue;
            case Types::Type::invalid: return TypedValue::invalidValue;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        TypedValue operator-() const
        {
            switch(m_type)
            {
            case Types::Type::i8: return -this->m_value_i8;
            case Types::Type::i16: return -this->m_value_i16;
            case Types::Type::i32: return -this->m_value_i32;
            case Types::Type::i64: return -this->m_value_i64;
            case Types::Type::f32: return -this->m_value_f32;
            case Types::Type::f64: return -this->m_value_f64;
            default: throw LINC_EXCEPTION_INVALID_INPUT("Invalid operand for unary negation");
            }
        }

        static bool stringToBool(const std::string& value)
        {
            if(value == LINC_TYPED_VALUE_LITERAL_TRUE)
                return true;
            else if(value == LINC_TYPED_VALUE_LITERAL_FALSE)
                return false;
            else
            {
                char* p;
                int64_t integral = strtoll(value.c_str(), &p, 10);
                
                if(*p)
                    throw LINC_EXCEPTION("Invalid boolean expression");
                
                return integral != 0;
            }
        }

        static TypedValue fromString(const std::string& value, Types::Type type)
        {
            switch(type)
            {
            case Types::Type::u8: return TypedValue(static_cast<Types::u8>(std::stoul(value)));
            case Types::Type::u16: return TypedValue(static_cast<Types::u16>(std::stoul(value)));
            case Types::Type::u32: return TypedValue(static_cast<Types::u32>(std::stoul(value)));
            case Types::Type::u64: return TypedValue(static_cast<Types::u64>(std::stoull(value)));
            case Types::Type::i8: return TypedValue(static_cast<Types::i8>(std::stol(value)));
            case Types::Type::i16: return TypedValue(static_cast<Types::i16>(std::stol(value)));
            case Types::Type::i32: return TypedValue(static_cast<Types::i32>(std::stol(value)));
            case Types::Type::i64: return TypedValue(static_cast<Types::i64>(std::stoll(value)));
            case Types::Type::f32: return TypedValue(static_cast<Types::f32>(std::stof(value)));
            case Types::Type::f64: return TypedValue(static_cast<Types::f64>(std::stod(value)));
            case Types::Type::_char: return TypedValue(static_cast<Types::_char>(value[0]));
            case Types::Type::_bool: return TypedValue(static_cast<Types::_bool>(stringToBool(value)));
            case Types::Type::string: return TypedValue(value);
            default: throw LINC_EXCEPTION("Type out of valid range in variant conversion");
            }
        }

        Types::string toString() const
        {
            switch(m_type)
            {
            case Types::Type::u8: return std::to_string(m_value_u8);
            case Types::Type::u16: return std::to_string(m_value_u16);
            case Types::Type::u32: return std::to_string(m_value_u32);
            case Types::Type::u64: return std::to_string(m_value_u64);
            case Types::Type::i8: return std::to_string(m_value_i8);
            case Types::Type::i16: return std::to_string(m_value_i16);
            case Types::Type::i32: return std::to_string(m_value_i32);
            case Types::Type::i64: return std::to_string(m_value_i64);
            case Types::Type::f32: return std::to_string(m_value_f32);
            case Types::Type::f64: return std::to_string(m_value_f64);
            case Types::Type::_char: return std::string(1, m_value__char);
            case Types::Type::_bool: return m_value__bool? LINC_TYPED_VALUE_LITERAL_TRUE: LINC_TYPED_VALUE_LITERAL_FALSE;
            case Types::Type::_void: return std::string{"{}"};
            case Types::Type::string: return m_value_string;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        inline Types::Type getType() const { return m_type; }

        static const TypedValue invalidValue, voidValue;
    private:
        TypedValue(Types::_void_type _void)
            :m_value__void(_void), m_type(Types::Type::_void)
        {}

        TypedValue(Types::invalid_type invalid)
            :m_value_invalid(invalid), m_type(Types::Type::invalid)
        {}

        union
        {
            Types::u8 m_value_u8;
            Types::u16 m_value_u16;
            Types::u32 m_value_u32;
            Types::u64 m_value_u64;
            Types::i8 m_value_i8;
            Types::i16 m_value_i16;
            Types::i32 m_value_i32;
            Types::i64 m_value_i64;
            Types::f32 m_value_f32;
            Types::f64 m_value_f64;
            Types::_char m_value__char;
            Types::_bool m_value__bool;
            Types::_void_type m_value__void;
            Types::invalid_type m_value_invalid;
            Types::string m_value_string;
        };
        Types::Type m_type;
    };
}