#pragma once
#include <variant>
#include <linc/system/Types.hpp>

#define LINC_TYPED_VALUE_CONSTRUCTOR(type) \
    TypedValue(Types::type value) \
        :m_value_##type(value), m_type(Types::Type::type) \
        {}

#define LINC_TYPED_VALUE_OPTIONAL_GETTER(type, type_fn_name) \
    std::optional<Types::type> getIf##type_fn_name() const \
    { \
        if(m_type == Types::Type::type) \
            return m_value_##type; \
        else return {}; \
    }

#define LINC_TYPED_VALUE_GETTER(type, type_fn_name) \
    Types::type get##type_fn_name() const \
    { \
        return m_value_##type; \
    }

namespace linc
{
    class TypedValue final
    {
    public:
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
            case Types::Type::_bool: return m_value__bool;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        template <typename T>
        TypedValue convert() const
        {
            switch(m_type)
            {
            case Types::Type::u8:    return static_cast<T>(m_value_u8);
            case Types::Type::u16:   return static_cast<T>(m_value_u16);
            case Types::Type::u32:   return static_cast<T>(m_value_u32);
            case Types::Type::u64:   return static_cast<T>(m_value_u64);
            case Types::Type::i8:    return static_cast<T>(m_value_i8);
            case Types::Type::i16:   return static_cast<T>(m_value_i16);
            case Types::Type::i32:   return static_cast<T>(m_value_i32);
            case Types::Type::i64:   return static_cast<T>(m_value_i64);
            case Types::Type::f32:   return static_cast<T>(m_value_f32);
            case Types::Type::f64:   return static_cast<T>(m_value_f64);
            case Types::Type::_char: return static_cast<T>(m_value__char);
            case Types::Type::_bool: return static_cast<T>(m_value__bool);
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
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        TypedValue operator+(const TypedValue& other) const
        {
            auto self = convert(other.m_type);

            switch(m_type)
            {
            case Types::Type::u8: return self.m_value_u8 + other.m_value_u8;
            case Types::Type::u16: return self.m_value_u16 + other.m_value_u16;
            case Types::Type::u32: return self.m_value_u32 + other.m_value_u32;
            case Types::Type::u64: return self.m_value_u64 + other.m_value_u64;
            case Types::Type::i8: return self.m_value_i8 + other.m_value_i8;
            case Types::Type::i16: return self.m_value_i16 + other.m_value_i16;
            case Types::Type::i32: return self.m_value_i32 + other.m_value_i32;
            case Types::Type::i64: return self.m_value_i64 + other.m_value_i64;
            case Types::Type::f32: return self.m_value_f32 + other.m_value_f32;
            case Types::Type::f64: return self.m_value_f64 + other.m_value_f64;
            case Types::Type::_char: return self.m_value__char + other.m_value__char;
            case Types::Type::_bool: return self.m_value__bool + other.m_value__bool;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        TypedValue operator-(const TypedValue& other) const
        {
            auto self = convert(other.m_type);

            switch(m_type)
            {
            case Types::Type::u8: return self.m_value_u8 - other.m_value_u8;
            case Types::Type::u16: return self.m_value_u16 - other.m_value_u16;
            case Types::Type::u32: return self.m_value_u32 - other.m_value_u32;
            case Types::Type::u64: return self.m_value_u64 - other.m_value_u64;
            case Types::Type::i8: return self.m_value_i8 - other.m_value_i8;
            case Types::Type::i16: return self.m_value_i16 - other.m_value_i16;
            case Types::Type::i32: return self.m_value_i32 - other.m_value_i32;
            case Types::Type::i64: return self.m_value_i64 - other.m_value_i64;
            case Types::Type::f32: return self.m_value_f32 - other.m_value_f32;
            case Types::Type::f64: return self.m_value_f64 - other.m_value_f64;
            case Types::Type::_char: return self.m_value__char - other.m_value__char;
            case Types::Type::_bool: return self.m_value__bool - other.m_value__bool;
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

        TypedValue operator*(const TypedValue& other) const
        {
            auto self = convert(other.m_type);

            switch(m_type)
            {
            case Types::Type::u8: return self.m_value_u8 * other.m_value_u8;
            case Types::Type::u16: return self.m_value_u16 * other.m_value_u16;
            case Types::Type::u32: return self.m_value_u32 * other.m_value_u32;
            case Types::Type::u64: return self.m_value_u64 * other.m_value_u64;
            case Types::Type::i8: return self.m_value_i8 * other.m_value_i8;
            case Types::Type::i16: return self.m_value_i16 * other.m_value_i16;
            case Types::Type::i32: return self.m_value_i32 * other.m_value_i32;
            case Types::Type::i64: return self.m_value_i64 * other.m_value_i64;
            case Types::Type::f32: return self.m_value_f32 * other.m_value_f32;
            case Types::Type::f64: return self.m_value_f64 * other.m_value_f64;
            case Types::Type::_char: return self.m_value__char * other.m_value__char;
            case Types::Type::_bool: return self.m_value__bool * other.m_value__bool;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        TypedValue operator/(const TypedValue& other) const
        {
            auto self = convert(other.m_type);

            switch(m_type)
            {
            case Types::Type::u8: return self.m_value_u8 / other.m_value_u8;
            case Types::Type::u16: return self.m_value_u16 / other.m_value_u16;
            case Types::Type::u32: return self.m_value_u32 / other.m_value_u32;
            case Types::Type::u64: return self.m_value_u64 / other.m_value_u64;
            case Types::Type::i8: return self.m_value_i8 / other.m_value_i8;
            case Types::Type::i16: return self.m_value_i16 / other.m_value_i16;
            case Types::Type::i32: return self.m_value_i32 / other.m_value_i32;
            case Types::Type::i64: return self.m_value_i64 / other.m_value_i64;
            case Types::Type::f32: return self.m_value_f32 / other.m_value_f32;
            case Types::Type::f64: return self.m_value_f64 / other.m_value_f64;
            case Types::Type::_char: return self.m_value__char / other.m_value__char;
            case Types::Type::_bool: return self.m_value__bool / other.m_value__bool;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        bool operator==(const TypedValue& other) const
        {
            auto self = convert(other.m_type);

            switch(m_type)
            {
            case Types::Type::u8: return self.m_value_u8 == other.m_value_u8;
            case Types::Type::u16: return self.m_value_u16 == other.m_value_u16;
            case Types::Type::u32: return self.m_value_u32 == other.m_value_u32;
            case Types::Type::u64: return self.m_value_u64 == other.m_value_u64;
            case Types::Type::i8: return self.m_value_i8 == other.m_value_i8;
            case Types::Type::i16: return self.m_value_i16 == other.m_value_i16;
            case Types::Type::i32: return self.m_value_i32 == other.m_value_i32;
            case Types::Type::i64: return self.m_value_i64 == other.m_value_i64;
            case Types::Type::f32: return self.m_value_f32 == other.m_value_f32;
            case Types::Type::f64: return self.m_value_f64 == other.m_value_f64;
            case Types::Type::_char: return self.m_value__char == other.m_value__char;
            case Types::Type::_bool: return self.m_value__bool == other.m_value__bool;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
            }
        }

        bool operator!=(const TypedValue& other) const
        {
            return !(*this == other);
        }

        static bool stringToBool(const std::string& value)
        {
            if(value == "true")
                return true;
            else if(value == "false")
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
            default: throw LINC_EXCEPTION("Type out of valid range in variant conversion");
            }
        }

        inline Types::Type getType() const { return m_type; }

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

        LINC_TYPED_VALUE_OPTIONAL_GETTER(u8, U8)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(u16, U16)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(u32, U32)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(u64, U64)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(i8, I8)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(i16, I16)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(i32, I32)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(i64, I64)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(f32, F32)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(f64, F64)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(_char, Char)
        LINC_TYPED_VALUE_OPTIONAL_GETTER(_bool, Bool)

        LINC_TYPED_VALUE_GETTER(u8, U8)
        LINC_TYPED_VALUE_GETTER(u16, U16)
        LINC_TYPED_VALUE_GETTER(u32, U32)
        LINC_TYPED_VALUE_GETTER(u64, U64)
        LINC_TYPED_VALUE_GETTER(i8, I8)
        LINC_TYPED_VALUE_GETTER(i16, I16)
        LINC_TYPED_VALUE_GETTER(i32, I32)
        LINC_TYPED_VALUE_GETTER(i64, I64)
        LINC_TYPED_VALUE_GETTER(f32, F32)
        LINC_TYPED_VALUE_GETTER(f64, F64)
        LINC_TYPED_VALUE_GETTER(_char, Char)
        LINC_TYPED_VALUE_GETTER(_bool, Bool)

    private:
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
        };
        Types::Type m_type;
    };
}