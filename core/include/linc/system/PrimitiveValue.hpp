#pragma once
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

#define LINC_PRIMITIVE_VALUE_LITERAL_TRUE "true"
#define LINC_PRIMITIVE_VALUE_LITERAL_FALSE "false"

#define LINC_PRIMITIVE_VALUE_CONSTRUCTOR(kind) \
    PrimitiveValue(Types::kind value) \
        :m_value_##kind(value), m_kind(Types::Kind::kind) \
        {}

#define LINC_PRIMITIVE_VALUE_GETTERS(type, type_fn_name, kind, value) \
    std::optional<Types::type> getIf##type_fn_name() const \
    { \
        if(m_kind == Kind::kind) \
            return m_value_##value; \
        else return std::nullopt; \
    } \
    Types::type get##type_fn_name() const \
    { \
        return m_value_##value; \
    }

#define LINC_PRIMITIVE_VALUE_OPERATOR_GENERIC(op, return_type) \
    return_type operator op(const PrimitiveValue& _other) const \
    { \
        auto other = _other.m_kind == m_kind? _other: _other.convert(m_kind); \
        switch(m_kind) \
        { \
        case Kind::Boolean: return m_value_bool op other.m_value_bool; \
        case Kind::Character: return m_value_char op other.m_value_char; \
        case Kind::Unsigned: return m_value_unsigned op other.m_value_unsigned; \
        case Kind::Signed: return m_value_signed op other.m_value_signed; \
        case Kind::Float: return m_value_float op other.m_value_float; \
        case Kind::Double: return m_value_double op other.m_value_double; \
        case Kind::String: return m_value_string op other.m_value_string; \
        case Kind::Type: return m_value_type op other.m_value_type; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind); \
        } \
    }

#define LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(op, return_type) \
    return_type operator op(const PrimitiveValue& _other) const \
    { \
        auto other = _other.m_kind == m_kind? _other: _other.convert(m_kind); \
        switch(m_kind) \
        { \
        case Kind::Boolean: return m_value_bool op other.m_value_bool; \
        case Kind::Character: return m_value_char op other.m_value_char; \
        case Kind::Unsigned: return m_value_unsigned op other.m_value_unsigned; \
        case Kind::Signed: return m_value_signed op other.m_value_signed; \
        case Kind::Float: return m_value_float op other.m_value_float; \
        case Kind::Double: return m_value_double op other.m_value_double; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind); \
        } \
    }

#define LINC_PRIMITIVE_VALUE_OPERATOR_UNARY_NUMERIC(op, return_type, const_keyword) \
return_type operator op() const_keyword \
    { \
        switch(m_kind) \
        { \
        case Kind::Unsigned: return op m_value_unsigned; \
        case Kind::Signed: return op m_value_signed; \
        case Kind::Float: return op m_value_float; \
        case Kind::Double: return op m_value_double; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind); \
        } \
    }

#define LINC_PRIMITIVE_VALUE_COPY_SWITCH \
    switch (m_kind) \
    { \
    case Kind::Void: m_value_void = other.m_value_void; break; \
    case Kind::Boolean: m_value_bool = other.m_value_bool; break; \
    case Kind::Character: m_value_char = other.m_value_char; break; \
    case Kind::Unsigned: m_value_unsigned = other.m_value_unsigned; break; \
    case Kind::Signed: m_value_signed = other.m_value_signed; break; \
    case Kind::Float: m_value_float = other.m_value_float; break; \
    case Kind::Double: m_value_double = other.m_value_double; break; \
    case Kind::String: new (&m_value_string) Types::string{other.m_value_string}; break; \
    case Kind::Type: m_value_type = other.m_value_type; break; \
    default: m_value_invalid = other.m_value_invalid; \
    }

#define LINC_PRIMITIVE_VALUE_MOVE_SWITCH \
    switch (m_kind) \
    { \
    case Kind::Void: m_value_void = other.m_value_void; break; \
    case Kind::Boolean: m_value_bool = other.m_value_bool; break; \
    case Kind::Character: m_value_char = other.m_value_char; break; \
    case Kind::Unsigned: m_value_unsigned = other.m_value_unsigned; break; \
    case Kind::Signed: m_value_signed = other.m_value_signed; break; \
    case Kind::Float: m_value_float = other.m_value_float; break; \
    case Kind::Double: m_value_double = other.m_value_double; break; \
    case Kind::String: new (&m_value_string) Types::string{std::move(other.m_value_string)}; break; \
    case Kind::Type: m_value_type = other.m_value_type; break; \
    default: m_value_invalid = other.m_value_invalid; \
    }

namespace linc
{
    class PrimitiveValue final
    {
    public:
        enum class Kind: unsigned char
        {
            Invalid, Void, Boolean, Character, Unsigned, Signed, Float, Double, String, Type
        };

        PrimitiveValue(Types::_bool value)
            :m_kind(Kind::Boolean), m_value_bool(value)
        {}

        PrimitiveValue(Types::_char value)
            :m_kind(Kind::Character), m_value_char(value)        
        {}

        PrimitiveValue(Types::u8 value)
            :m_kind(Kind::Unsigned), m_value_unsigned(value)
        {}
        
        PrimitiveValue(Types::i8 value)
            :m_kind(Kind::Signed), m_value_signed(value)
        {}

        PrimitiveValue(Types::u16 value)
            :m_kind(Kind::Unsigned), m_value_unsigned(value)
        {}
        
        PrimitiveValue(Types::i16 value)
            :m_kind(Kind::Signed), m_value_signed(value)
        {}

        PrimitiveValue(Types::u32 value)
            :m_kind(Kind::Unsigned), m_value_unsigned(value)
        {}
        
        PrimitiveValue(Types::i32 value)
            :m_kind(Kind::Signed), m_value_signed(value)
        {}

        PrimitiveValue(Types::u64 value)
            :m_kind(Kind::Unsigned), m_value_unsigned(value)
        {}
        
        PrimitiveValue(Types::i64 value)
            :m_kind(Kind::Signed), m_value_signed(value)
        {}

        PrimitiveValue(Types::f32 value)
            :m_kind(Kind::Float), m_value_float(value)
        {}

        PrimitiveValue(Types::f64 value)
            :m_kind(Kind::Double), m_value_double(value)
        {}

        PrimitiveValue(const Types::string& value)
            :m_kind(Kind::String)
        {
            new (&m_value_string) std::string{value};
        }

        PrimitiveValue(const Types::type& value)
            :m_kind(Kind::Type), m_value_type(value)
        {}

        ~PrimitiveValue()
        {
            if(m_kind == Kind::String)
                m_value_string.~basic_string();
        }

        PrimitiveValue(PrimitiveValue&& other)
            :m_kind(other.m_kind)
        {
            other.m_kind = Kind::Invalid; 

            LINC_PRIMITIVE_VALUE_MOVE_SWITCH
        }

        PrimitiveValue(const PrimitiveValue& other)
            :m_kind(other.m_kind)
        {
            LINC_PRIMITIVE_VALUE_COPY_SWITCH
        }

        static PrimitiveValue fromDefault(Types::Kind kind)
        {
            switch(kind)
            {
            case Types::Kind::invalid: return PrimitiveValue::invalidValue;
            case Types::Kind::_void: return PrimitiveValue::voidValue;
            case Types::Kind::_bool: return false;
            case Types::Kind::_char: return '\0';
            case Types::Kind::u8:
            case Types::Kind::u16:
            case Types::Kind::u32:
            case Types::Kind::u64:
                return 0u;
            case Types::Kind::i8:
            case Types::Kind::i16:
            case Types::Kind::i32:
            case Types::Kind::i64:
                return 0;
            case Types::Kind::f32: return 0.0f;
            case Types::Kind::f64: return 0.0;
            case Types::Kind::string: return "";
            case Types::Kind::type: return Types::voidType;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }

        PrimitiveValue& operator=(const PrimitiveValue& other)
        {
            m_kind = other.m_kind;

            LINC_PRIMITIVE_VALUE_COPY_SWITCH

            return *this;
        }

        PrimitiveValue& operator=(PrimitiveValue&& other)
        {
            m_kind = other.m_kind;
            other.m_kind = Kind::Invalid; 

            LINC_PRIMITIVE_VALUE_MOVE_SWITCH

            other.m_value_invalid = Types::_invalid_type{};
            return *this;
        }

        bool isZero()
        {
            switch (m_kind)
            {
            case Kind::Unsigned: return m_value_unsigned == 0; 
            case Kind::Signed: return m_value_signed == 0;
            case Kind::Float: return m_value_float == 0;
            case Kind::Double: return m_value_double == 0;
            case Kind::Character: return m_value_char == 0;
            case Kind::Boolean: return !m_value_bool;
            case Kind::Void: return true;
            default: return false;
            }
        }

        LINC_PRIMITIVE_VALUE_OPERATOR_UNARY_NUMERIC(++, PrimitiveValue, )
        LINC_PRIMITIVE_VALUE_OPERATOR_UNARY_NUMERIC(--, PrimitiveValue, )
        LINC_PRIMITIVE_VALUE_OPERATOR_UNARY_NUMERIC(-, PrimitiveValue, const)
        LINC_PRIMITIVE_VALUE_OPERATOR_UNARY_NUMERIC(+, PrimitiveValue, const)

        LINC_PRIMITIVE_VALUE_GETTERS(_void_type, Void, Void, void)
        LINC_PRIMITIVE_VALUE_GETTERS(_bool, Bool, Boolean, bool)
        LINC_PRIMITIVE_VALUE_GETTERS(_char, Char, Character, char)

        LINC_PRIMITIVE_VALUE_GETTERS(f32, F32, Float, float)
        LINC_PRIMITIVE_VALUE_GETTERS(f64, F64, Double, double)

        LINC_PRIMITIVE_VALUE_GETTERS(u8, U8, Unsigned, unsigned)
        LINC_PRIMITIVE_VALUE_GETTERS(u16, U16, Unsigned, unsigned)
        LINC_PRIMITIVE_VALUE_GETTERS(u32, U32, Unsigned, unsigned)
        LINC_PRIMITIVE_VALUE_GETTERS(u64, U64, Unsigned, unsigned)

        LINC_PRIMITIVE_VALUE_GETTERS(i8, I8, Signed, signed)
        LINC_PRIMITIVE_VALUE_GETTERS(i16, I16, Signed, signed)
        LINC_PRIMITIVE_VALUE_GETTERS(i32, I32, Signed, signed)
        LINC_PRIMITIVE_VALUE_GETTERS(i64, I64, Signed, signed)

        LINC_PRIMITIVE_VALUE_GETTERS(string, String, String, string)
        LINC_PRIMITIVE_VALUE_GETTERS(type, Type, Type, type)

        LINC_PRIMITIVE_VALUE_OPERATOR_GENERIC(==, bool)
        LINC_PRIMITIVE_VALUE_OPERATOR_GENERIC(!=, bool)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(-, PrimitiveValue)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(*, PrimitiveValue)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(/, PrimitiveValue)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(>, bool)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(<, bool)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(>=, bool)
        LINC_PRIMITIVE_VALUE_OPERATOR_NUMERIC(<=, bool)

        PrimitiveValue operator+(const PrimitiveValue& _other) const
        {
            if((m_kind == Kind::Character && _other.m_kind == Kind::String)
            || (m_kind == Kind::String && _other.m_kind == Kind::Character)
            || (m_kind == Kind::Character && _other.m_kind == Kind::Character))
                return toString() + _other.toString();
            
            auto other = _other.convert(m_kind);

            switch(m_kind)
            {
            case Kind::Unsigned: return m_value_unsigned + other.m_value_unsigned;
            case Kind::Signed: return m_value_signed + other.m_value_signed;
            case Kind::Float: return m_value_float + other.m_value_float;
            case Kind::Double: return m_value_double + other.m_value_double;
            case Kind::String: return m_value_string + other.m_value_string;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }

        Types::Variant toVariant() const
        {
            switch(m_kind)
            {
            case Kind::Invalid: return m_value_invalid;
            case Kind::Void: return m_value_void;
            case Kind::Boolean: return m_value_bool;
            case Kind::Character: return m_value_char;
            case Kind::Unsigned: return m_value_unsigned;
            case Kind::Signed: return m_value_signed;
            case Kind::Float: return m_value_float;
            case Kind::Double: return m_value_double;
            case Kind::String: return m_value_string;
            case Kind::Type: return m_value_type;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }

        PrimitiveValue convert(Kind kind) const
        {
            switch(kind)
            {
            case Kind::Invalid: return invalidValue;
            case Kind::Void: return voidValue;
            case Kind::Boolean: return convert<Types::_bool>();
            case Kind::Character: return convert<Types::_char>();
            case Kind::Unsigned: return convert<Types::u64>();
            case Kind::Signed: return convert<Types::i64>();
            case Kind::Float: return convert<Types::f32>();
            case Kind::Double: return convert<Types::f64>();
            case Kind::String: return toString();
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }

        PrimitiveValue convert(Types::Kind kind) const
        {
            switch(kind)
            {
            case Types::Kind::invalid: return invalidValue;
            case Types::Kind::_void: return voidValue;
            case Types::Kind::_bool: return convert<Types::_bool>();
            case Types::Kind::_char: return convert<Types::_char>();
            case Types::Kind::u8: return convert<Types::u8>();
            case Types::Kind::u16: return convert<Types::u16>();
            case Types::Kind::u32: return convert<Types::u32>();
            case Types::Kind::u64: return convert<Types::u64>();
            case Types::Kind::i8: return convert<Types::i8>();
            case Types::Kind::i16: return convert<Types::i16>();
            case Types::Kind::i32: return convert<Types::i32>();
            case Types::Kind::i64: return convert<Types::i64>();
            case Types::Kind::f32: return convert<Types::f32>();
            case Types::Kind::f64: return convert<Types::f64>();
            case Types::Kind::string: return toString();
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        template <typename T>
        T convert() const
        {
            switch(m_kind)
            {
            case Kind::Invalid:
            case Kind::Void:
                return static_cast<T>(0);
            case Kind::Boolean: return static_cast<T>(m_value_bool);
            case Kind::Character: return static_cast<T>(m_value_char);
            case Kind::Unsigned: return static_cast<T>(m_value_unsigned);
            case Kind::Signed: return static_cast<T>(m_value_signed);
            case Kind::Float: return static_cast<T>(m_value_float);
            case Kind::Double: return static_cast<T>(m_value_double);
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }

        static bool stringToBool(const std::string& value)
        {
            if(value == LINC_PRIMITIVE_VALUE_LITERAL_TRUE)
                return true;
            else if(value == LINC_PRIMITIVE_VALUE_LITERAL_FALSE)
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

        static PrimitiveValue fromString(const std::string& value, Types::Kind type)
        {
            switch(type)
            {
            case Types::Kind::u8: return PrimitiveValue(static_cast<Types::u8>(std::stoul(value)));
            case Types::Kind::u16: return PrimitiveValue(static_cast<Types::u16>(std::stoul(value)));
            case Types::Kind::u32: return PrimitiveValue(static_cast<Types::u32>(std::stoul(value)));
            case Types::Kind::u64: return PrimitiveValue(static_cast<Types::u64>(std::stoull(value)));
            case Types::Kind::i8: return PrimitiveValue(static_cast<Types::i8>(std::stol(value)));
            case Types::Kind::i16: return PrimitiveValue(static_cast<Types::i16>(std::stol(value)));
            case Types::Kind::i32: return PrimitiveValue(static_cast<Types::i32>(std::stol(value)));
            case Types::Kind::i64: return PrimitiveValue(static_cast<Types::i64>(std::stoll(value)));
            case Types::Kind::f32: return PrimitiveValue(static_cast<Types::f32>(std::stof(value)));
            case Types::Kind::f64: return PrimitiveValue(static_cast<Types::f64>(std::stod(value)));
            case Types::Kind::_char: return PrimitiveValue(static_cast<Types::_char>(value[0]));
            case Types::Kind::_bool: return PrimitiveValue(static_cast<Types::_bool>(stringToBool(value)));
            case Types::Kind::string: return PrimitiveValue(value);
            default: throw LINC_EXCEPTION("Type out of valid range in variant conversion");
            }
        }

        Types::string toString() const
        {
            switch(m_kind)
            {
            case Kind::Invalid: return "";
            case Kind::Void: return "{}";
            case Kind::Boolean: return m_value_bool? LINC_PRIMITIVE_VALUE_LITERAL_TRUE: LINC_PRIMITIVE_VALUE_LITERAL_FALSE;
            case Kind::Character: return Types::string(1, m_value_char);
            case Kind::Unsigned: return std::to_string(m_value_unsigned);
            case Kind::Signed: return std::to_string(m_value_signed);
            case Kind::Float: return std::to_string(m_value_float);
            case Kind::Double: return std::to_string(m_value_double);
            case Kind::String: return m_value_string;
            case Kind::Type: return Types::toString(m_value_type);
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(PrimitiveValue::Kind);
            }
        }
        inline Kind getKind() const { return m_kind; }

        static const PrimitiveValue invalidValue, voidValue;
    private:
        PrimitiveValue(Types::_invalid_type value)
            :m_value_invalid(value), m_kind(Kind::Invalid)
        {}

        PrimitiveValue(Types::_void_type value)
            :m_value_void(value), m_kind(Kind::Void)
        {}

        union
        {
            Types::_invalid_type m_value_invalid;
            Types::_void_type m_value_void;
            Types::_bool m_value_bool;
            Types::_char m_value_char;
            Types::u64 m_value_unsigned;
            Types::i64 m_value_signed;
            Types::f32 m_value_float;
            Types::f64 m_value_double;
            Types::string m_value_string;
            Types::type m_value_type;
        };
        Kind m_kind;
    };
}