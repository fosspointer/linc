#pragma once
#include <linc/system/Exception.hpp>
#include <linc/system/PrimitiveValue.hpp>
#include <linc/Include.hpp>
#include <cstring>
#include <cstdlib>

#define LINC_ARRAY_TYPE_FUNCTIONS(type, type_fn) \
    LINC_ARRAY_VALUE_CONSTRUCTOR(type) \
    LINC_ARRAY_VALUE_PUSH(type, type_fn)

#define LINC_ARRAY_VALUE_CONSTRUCTOR(type) \
    ArrayValue(std::vector<Types::type> array) \
        :m_kind(Types::Kind::type) \
    { \
        new (&m_array_##type) std::vector{std::move(array)}; \
    }

#define LINC_ARRAY_VALUE_PUSH(type, type_fn) \
    void push##type_fn(Types::type value) \
    { \
        if(m_kind != Types::Kind::type) \
            throw LINC_EXCEPTION_INVALID_INPUT("Cannot push value of incorrect type to array value."); \
        m_array_##type.push_back(std::move(value)); \
    }
namespace linc
{
    class ArrayValue final 
    {
    public:
        ArrayValue(std::vector<Types::_invalid_type> array)
            :m_kind(Types::Kind::invalid)
        {
            new (&m_array_invalid) std::vector{std::move(array)};
        }

        ArrayValue(std::vector<Types::_void_type> array)
            :m_kind(Types::Kind::_void)
        {
            new (&m_array__void) std::vector{std::move(array)};
        }

        LINC_ARRAY_TYPE_FUNCTIONS(_bool, Bool)
        LINC_ARRAY_TYPE_FUNCTIONS(_char, Char)
        LINC_ARRAY_TYPE_FUNCTIONS(u8, U8)
        LINC_ARRAY_TYPE_FUNCTIONS(u16, U16)
        LINC_ARRAY_TYPE_FUNCTIONS(u32, U32)
        LINC_ARRAY_TYPE_FUNCTIONS(u64, U64)
        LINC_ARRAY_TYPE_FUNCTIONS(i8, I8)
        LINC_ARRAY_TYPE_FUNCTIONS(i16, I16)
        LINC_ARRAY_TYPE_FUNCTIONS(i32, I32)
        LINC_ARRAY_TYPE_FUNCTIONS(i64, I64)
        LINC_ARRAY_TYPE_FUNCTIONS(f32, F32)
        LINC_ARRAY_TYPE_FUNCTIONS(f64, F64)
        LINC_ARRAY_TYPE_FUNCTIONS(string, String)
        LINC_ARRAY_TYPE_FUNCTIONS(type, Type)

        inline Types::Kind getKind() const { return m_kind; }

        void pushVoid(Types::_void_type value)
        {
            if(m_kind != Types::Kind::_void)
                throw LINC_EXCEPTION_INVALID_INPUT("Cannot push value of incorrect type to array value.");
            m_array__void.push_back(std::move(value));
        }

        void push(const PrimitiveValue& value)
        {
            switch(m_kind)
            {
            case Types::Kind::_void: pushVoid(value.getIfVoid().value()); break;
            case Types::Kind::_bool: pushBool(value.getIfBool().value()); break;
            case Types::Kind::_char: pushChar(value.getIfChar().value()); break;
            case Types::Kind::u8: pushU8(value.getIfU8().value()); break;
            case Types::Kind::u16: pushU16(value.getIfU16().value()); break;
            case Types::Kind::u32: pushU32(value.getIfU32().value()); break;
            case Types::Kind::u64: pushU64(value.getIfU64().value()); break;
            case Types::Kind::i8: pushI8(value.getIfI8().value()); break;
            case Types::Kind::i16: pushI16(value.getIfI16().value()); break;
            case Types::Kind::i32: pushI32(value.getIfI32().value()); break;
            case Types::Kind::i64: pushI64(value.getIfI64().value()); break;
            case Types::Kind::f32: pushF32(value.getIfF32().value()); break;
            case Types::Kind::f64: pushF64(value.getIfF64().value()); break;
            case Types::Kind::string: pushString(value.getIfString().value()); break;
            case Types::Kind::type: pushType(value.getIfType().value()); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        ~ArrayValue()
        {
            switch(m_kind)
            {
            case Types::Kind::invalid: m_array_invalid.~vector(); break;
            case Types::Kind::_void: m_array__void.~vector(); break;
            case Types::Kind::_bool: m_array__bool.~vector(); break;
            case Types::Kind::_char: m_array__char.~vector(); break;
            case Types::Kind::u8: m_array_u8.~vector(); break;
            case Types::Kind::u16: m_array_u16.~vector(); break;
            case Types::Kind::u32: m_array_u32.~vector(); break;
            case Types::Kind::u64: m_array_u64.~vector(); break;
            case Types::Kind::i8: m_array_i8.~vector(); break;
            case Types::Kind::i16: m_array_i16.~vector(); break;
            case Types::Kind::i32: m_array_i32.~vector(); break;
            case Types::Kind::i64: m_array_i64.~vector(); break;
            case Types::Kind::f32: m_array_f32.~vector(); break;
            case Types::Kind::f64: m_array_f64.~vector(); break;
            case Types::Kind::string: m_array_string.~vector(); break;
            case Types::Kind::type: m_array_type.~vector(); break;
            }
        }

        ArrayValue(const ArrayValue& other)
            :m_kind(other.m_kind)
        {
            switch(m_kind)
            {
            case Types::Kind::_void: new (&m_array__void) std::vector{other.m_array__void}; break;
            case Types::Kind::_bool: new (&m_array__bool) std::vector{other.m_array__bool}; break;
            case Types::Kind::_char: new (&m_array__char) std::vector{other.m_array__char}; break;
            case Types::Kind::u8: new (&m_array_u8) std::vector{other.m_array_u8}; break;
            case Types::Kind::u16: new (&m_array_u16) std::vector{other.m_array_u16}; break;
            case Types::Kind::u32: new (&m_array_u32) std::vector{other.m_array_u32}; break;
            case Types::Kind::u64: new (&m_array_u64) std::vector{other.m_array_u64}; break;
            case Types::Kind::i8: new (&m_array_i8) std::vector{other.m_array_i8}; break;
            case Types::Kind::i16: new (&m_array_i16) std::vector{other.m_array_i16}; break;
            case Types::Kind::i32: new (&m_array_i32) std::vector{other.m_array_i32}; break;
            case Types::Kind::i64: new (&m_array_i64) std::vector{other.m_array_i64}; break;
            case Types::Kind::f32: new (&m_array_f32) std::vector{other.m_array_f32}; break;
            case Types::Kind::f64: new (&m_array_f64) std::vector{other.m_array_f64}; break;
            case Types::Kind::string: new (&m_array_string) std::vector{other.m_array_string}; break;
            case Types::Kind::type: new (&m_array_type) std::vector{other.m_array_type}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        ArrayValue(ArrayValue&& other)
            :m_kind(other.m_kind)
        {
            other.m_kind = Types::Kind::invalid;

            switch(m_kind)
            {
            case Types::Kind::_void: new (&m_array__void) std::vector{std::move(other.m_array__void)}; break;
            case Types::Kind::_bool: new (&m_array__bool) std::vector{std::move(other.m_array__bool)}; break;
            case Types::Kind::_char: new (&m_array__char) std::vector{std::move(other.m_array__char)}; break;
            case Types::Kind::u8: new (&m_array_u8) std::vector{std::move(other.m_array_u8)}; break;
            case Types::Kind::u16: new (&m_array_u16) std::vector{std::move(other.m_array_u16)}; break;
            case Types::Kind::u32: new (&m_array_u32) std::vector{std::move(other.m_array_u32)}; break;
            case Types::Kind::u64: new (&m_array_u64) std::vector{std::move(other.m_array_u64)}; break;
            case Types::Kind::i8: new (&m_array_i8) std::vector{std::move(other.m_array_i8)}; break;
            case Types::Kind::i16: new (&m_array_i16) std::vector{std::move(other.m_array_i16)}; break;
            case Types::Kind::i32: new (&m_array_i32) std::vector{std::move(other.m_array_i32)}; break;
            case Types::Kind::i64: new (&m_array_i64) std::vector{std::move(other.m_array_i64)}; break;
            case Types::Kind::f32: new (&m_array_f32) std::vector{std::move(other.m_array_f32)}; break;
            case Types::Kind::f64: new (&m_array_f64) std::vector{std::move(other.m_array_f64)}; break;
            case Types::Kind::string: new (&m_array_string) std::vector{std::move(other.m_array_string)}; break;
            case Types::Kind::type: new (&m_array_type) std::vector{std::move(other.m_array_type)}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }

            other.m_array_invalid = {std::vector<Types::_invalid_type>{}};
        }

        ArrayValue& operator=(const ArrayValue& other)
        {
            m_kind = other.m_kind;

            switch(m_kind)
            {
            case Types::Kind::_void: new (&m_array__void) std::vector{other.m_array__void}; break;
            case Types::Kind::_bool: new (&m_array__bool) std::vector{other.m_array__bool}; break;
            case Types::Kind::_char: new (&m_array__char) std::vector{other.m_array__char}; break;
            case Types::Kind::u8: new (&m_array_u8) std::vector{other.m_array_u8}; break;
            case Types::Kind::u16: new (&m_array_u16) std::vector{other.m_array_u16}; break;
            case Types::Kind::u32: new (&m_array_u32) std::vector{other.m_array_u32}; break;
            case Types::Kind::u64: new (&m_array_u64) std::vector{other.m_array_u64}; break;
            case Types::Kind::i8: new (&m_array_i8) std::vector{other.m_array_i8}; break;
            case Types::Kind::i16: new (&m_array_i16) std::vector{other.m_array_i16}; break;
            case Types::Kind::i32: new (&m_array_i32) std::vector{other.m_array_i32}; break;
            case Types::Kind::i64: new (&m_array_i64) std::vector{other.m_array_i64}; break;
            case Types::Kind::f32: new (&m_array_f32) std::vector{other.m_array_f32}; break;
            case Types::Kind::f64: new (&m_array_f64) std::vector{other.m_array_f64}; break;
            case Types::Kind::string: new (&m_array_string) std::vector{other.m_array_string}; break;
            case Types::Kind::type: new (&m_array_type) std::vector{other.m_array_type}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }

            return *this;
        }

        ArrayValue& operator=(ArrayValue&& other)
        {
            m_kind = other.m_kind;
            other.m_kind = Types::Kind::invalid;

            switch(m_kind)
            {
            case Types::Kind::_void: new (&m_array__void) std::vector{std::move(other.m_array__void)}; break;
            case Types::Kind::_bool: new (&m_array__bool) std::vector{std::move(other.m_array__bool)}; break;
            case Types::Kind::_char: new (&m_array__char) std::vector{std::move(other.m_array__char)}; break;
            case Types::Kind::u8: new (&m_array_u8) std::vector{std::move(other.m_array_u8)}; break;
            case Types::Kind::u16: new (&m_array_u16) std::vector{std::move(other.m_array_u16)}; break;
            case Types::Kind::u32: new (&m_array_u32) std::vector{std::move(other.m_array_u32)}; break;
            case Types::Kind::u64: new (&m_array_u64) std::vector{std::move(other.m_array_u64)}; break;
            case Types::Kind::i8: new (&m_array_i8) std::vector{std::move(other.m_array_i8)}; break;
            case Types::Kind::i16: new (&m_array_i16) std::vector{std::move(other.m_array_i16)}; break;
            case Types::Kind::i32: new (&m_array_i32) std::vector{std::move(other.m_array_i32)}; break;
            case Types::Kind::i64: new (&m_array_i64) std::vector{std::move(other.m_array_i64)}; break;
            case Types::Kind::f32: new (&m_array_f32) std::vector{std::move(other.m_array_f32)}; break;
            case Types::Kind::f64: new (&m_array_f64) std::vector{std::move(other.m_array_f64)}; break;
            case Types::Kind::string: new (&m_array_string) std::vector{std::move(other.m_array_string)}; break;
            case Types::Kind::type: new (&m_array_type) std::vector{std::move(other.m_array_type)}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }

            other.m_array_invalid = {std::vector<Types::_invalid_type>{}};
            return *this;
        }

        static ArrayValue fromDefault(Types::Kind kind)
        {
            switch(kind)
            {
            case Types::Kind::_void: return ArrayValue(std::vector<Types::_void_type>{});
            case Types::Kind::_bool: return ArrayValue(std::vector<Types::_bool>{});
            case Types::Kind::_char: return ArrayValue(std::vector<Types::_char>{});
            case Types::Kind::u8: return ArrayValue(std::vector<Types::u8>{});
            case Types::Kind::u16: return ArrayValue(std::vector<Types::u16>{});
            case Types::Kind::u32: return ArrayValue(std::vector<Types::u32>{});
            case Types::Kind::u64: return ArrayValue(std::vector<Types::u64>{});
            case Types::Kind::i8: return ArrayValue(std::vector<Types::i8>{});
            case Types::Kind::i16: return ArrayValue(std::vector<Types::i16>{});
            case Types::Kind::i32: return ArrayValue(std::vector<Types::i32>{});
            case Types::Kind::i64: return ArrayValue(std::vector<Types::i64>{});
            case Types::Kind::f32: return ArrayValue(std::vector<Types::f32>{});
            case Types::Kind::f64: return ArrayValue(std::vector<Types::f64>{});
            case Types::Kind::string: return ArrayValue(std::vector<Types::string>{});
            case Types::Kind::type: return ArrayValue(std::vector<Types::type>{});
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        Types::u64 getCount() const 
        {
            switch(m_kind)
            {
            case Types::Kind::_void: return m_array__void.size();
            case Types::Kind::_bool: return m_array__bool.size();
            case Types::Kind::_char: return m_array__char.size();
            case Types::Kind::u8: return m_array_u8.size();
            case Types::Kind::u16: return m_array_u16.size();
            case Types::Kind::u32: return m_array_u32.size();
            case Types::Kind::u64: return m_array_u64.size();
            case Types::Kind::i8: return m_array_i8.size();
            case Types::Kind::i16: return m_array_i16.size();
            case Types::Kind::i32: return m_array_i32.size();
            case Types::Kind::i64: return m_array_i64.size();
            case Types::Kind::f32: return m_array_f32.size();
            case Types::Kind::f64: return m_array_f64.size();
            case Types::Kind::string: return m_array_string.size();
            case Types::Kind::type: return m_array_type.size();
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        PrimitiveValue get(std::size_t index) const
        {
            switch(m_kind)
            {
            case Types::Kind::_void: return PrimitiveValue::voidValue;
            case Types::Kind::_bool: return m_array__bool[index];
            case Types::Kind::_char: return m_array__char[index];
            case Types::Kind::u8: return m_array_u8[index];
            case Types::Kind::u16: return m_array_u16[index];
            case Types::Kind::u32: return m_array_u32[index];
            case Types::Kind::u64: return m_array_u64[index];
            case Types::Kind::i8: return m_array_i8[index];
            case Types::Kind::i16: return m_array_i16[index];
            case Types::Kind::i32: return m_array_i32[index];
            case Types::Kind::i64: return m_array_i64[index];
            case Types::Kind::f32: return m_array_f32[index];
            case Types::Kind::f64: return m_array_f64[index];
            case Types::Kind::string: return m_array_string[index];
            case Types::Kind::type: return m_array_type[index];
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        void set(std::size_t index, const PrimitiveValue& value)
        {
            switch(m_kind)
            {
            case Types::Kind::_void: m_array__void[index] = value.getVoid(); break;
            case Types::Kind::_bool: m_array__bool[index] = value.getBool(); break;
            case Types::Kind::_char: m_array__char[index] = value.getChar(); break;
            case Types::Kind::u8: m_array_u8[index] = value.getU8(); break;
            case Types::Kind::u16: m_array_u16[index] = value.getU16(); break;
            case Types::Kind::u32: m_array_u32[index] = value.getU32(); break;
            case Types::Kind::u64: m_array_u64[index] = value.getU64(); break;
            case Types::Kind::i8: m_array_i8[index] = value.getI8(); break;
            case Types::Kind::i16: m_array_i16[index] = value.getI16(); break;
            case Types::Kind::i32: m_array_i32[index] = value.getI32(); break;
            case Types::Kind::i64: m_array_i64[index] = value.getI64(); break;
            case Types::Kind::f32: m_array_f32[index] = value.getF32(); break;
            case Types::Kind::f64: m_array_f64[index] = value.getF64(); break;
            case Types::Kind::string: m_array_string[index] = value.getString(); break;
            case Types::Kind::type: m_array_type[index] = value.getType(); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        ArrayValue operator+(const ArrayValue& other) const
        {
            switch(m_kind)
            {
            case Types::Kind::_void:{ auto result = m_array__void; result.insert(result.end(), other.m_array__void.begin(), other.m_array__void.end());
                return ArrayValue(result); }
            case Types::Kind::_bool:{ auto result = m_array__bool; result.insert(result.end(), other.m_array__bool.begin(), other.m_array__bool.end());
                return ArrayValue(result); }
            case Types::Kind::_char:{ auto result = m_array__char; result.insert(result.end(), other.m_array__char.begin(), other.m_array__char.end());
                return ArrayValue(result); }
            case Types::Kind::u8:{ auto result = m_array_u8; result.insert(result.end(), other.m_array_u8.begin(), other.m_array_u8.end());
                return ArrayValue(result); }
            case Types::Kind::u16:{ auto result = m_array_u16; result.insert(result.end(), other.m_array_u16.begin(), other.m_array_u16.end());
                return ArrayValue(result); }
            case Types::Kind::u32:{ auto result = m_array_u32; result.insert(result.end(), other.m_array_u32.begin(), other.m_array_u32.end());
                return ArrayValue(result); }
            case Types::Kind::u64:{ auto result = m_array_u64; result.insert(result.end(), other.m_array_u64.begin(), other.m_array_u64.end());
                return ArrayValue(result); }
            case Types::Kind::i8:{ auto result = m_array_i8; result.insert(result.end(), other.m_array_i8.begin(), other.m_array_i8.end());
                return ArrayValue(result); }
            case Types::Kind::i16:{ auto result = m_array_i16; result.insert(result.end(), other.m_array_i16.begin(), other.m_array_i16.end());
                return ArrayValue(result); }
            case Types::Kind::i32:{ auto result = m_array_i32; result.insert(result.end(), other.m_array_i32.begin(), other.m_array_i32.end());
                return ArrayValue(result); }
            case Types::Kind::i64:{ auto result = m_array_i64; result.insert(result.end(), other.m_array_i64.begin(), other.m_array_i64.end());
                return ArrayValue(result); }
            case Types::Kind::f32:{ auto result = m_array_f32; result.insert(result.end(), other.m_array_f32.begin(), other.m_array_f32.end());
                return ArrayValue(result); }
            case Types::Kind::f64:{ auto result = m_array_f64; result.insert(result.end(), other.m_array_f64.begin(), other.m_array_f64.end());
                return ArrayValue(result); }
            case Types::Kind::string:{ auto result = m_array_string; result.insert(result.end(), other.m_array_string.begin(), other.m_array_string.end());
                return ArrayValue(result); }
            case Types::Kind::type:{ auto result = m_array_type; result.insert(result.end(), other.m_array_type.begin(), other.m_array_type.end());
                return ArrayValue(result); }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        bool operator==(const ArrayValue& other) const
        {
            if(m_kind != other.m_kind)
                return false;

            switch(m_kind)
            {
            case Types::Kind::_void: return m_array__void.size() == other.m_array__void.size();
            case Types::Kind::_bool: return m_array__bool == other.m_array__bool;
            case Types::Kind::_char: return m_array__char == other.m_array__char;
            case Types::Kind::u8: return m_array_u8 == other.m_array_u8;
            case Types::Kind::u16: return m_array_u16 == other.m_array_u16;
            case Types::Kind::u32: return m_array_u32 == other.m_array_u32;
            case Types::Kind::u64: return m_array_u64 == other.m_array_u64;
            case Types::Kind::i8: return m_array_i8 == other.m_array_i8;
            case Types::Kind::i16: return m_array_i16 == other.m_array_i16;
            case Types::Kind::i32: return m_array_i32 == other.m_array_i32;
            case Types::Kind::i64: return m_array_i64 == other.m_array_i64;
            case Types::Kind::f32: return m_array_f32 == other.m_array_f32;
            case Types::Kind::f64: return m_array_f64 == other.m_array_f64;
            case Types::Kind::string: return m_array_string == other.m_array_string;
            case Types::Kind::type: return m_array_type == other.m_array_type;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }
        }

        bool operator!=(const ArrayValue& other) const
        {
            return !(*this == other);
        }

        std::vector<PrimitiveValue> toPrimitiveList() const 
        {
            std::vector<PrimitiveValue> result;

            switch(m_kind)
            {
            case Types::Kind::_void: for(const auto& value: m_array__void) result.push_back(PrimitiveValue::voidValue); break;
            case Types::Kind::_bool: for(const auto& value: m_array__bool) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::_char: for(const auto& value: m_array__char) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::u8: for(const auto& value: m_array_u8) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::u16: for(const auto& value: m_array_u16) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::u32: for(const auto& value: m_array_u32) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::u64: for(const auto& value: m_array_u64) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::i8: for(const auto& value: m_array_i8) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::i16: for(const auto& value: m_array_i16) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::i32: for(const auto& value: m_array_i32) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::i64: for(const auto& value: m_array_i64) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::f32: for(const auto& value: m_array_f32) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::f64: for(const auto& value: m_array_f64) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::string: for(const auto& value: m_array_string) result.push_back(PrimitiveValue(value)); break;
            case Types::Kind::type: for(const auto& value: m_array_type) result.push_back(PrimitiveValue(value)); break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Kind);
            }

            return result;
        }

        std::string toString() const 
        {
            std::string result;
            result.push_back('[');

            auto list = toPrimitiveList();
            for(std::size_t i = 0ull; i < list.size(); ++i)
            {
                result.append(list[i].toString());

                if(i != list.size() - 1)
                    result.append(", ");
            }

            result.push_back(']');
            return result;
        }

    private:
        union
        {
            std::vector<Types::_invalid_type> m_array_invalid;
            std::vector<Types::_void_type> m_array__void;
            std::vector<Types::_bool> m_array__bool;
            std::vector<Types::_char> m_array__char;
            std::vector<Types::u8> m_array_u8;
            std::vector<Types::u16> m_array_u16;
            std::vector<Types::u32> m_array_u32;
            std::vector<Types::u64> m_array_u64;
            std::vector<Types::i8> m_array_i8;
            std::vector<Types::i16> m_array_i16;
            std::vector<Types::i32> m_array_i32;
            std::vector<Types::i64> m_array_i64;
            std::vector<Types::f32> m_array_f32;
            std::vector<Types::f64> m_array_f64;
            std::vector<Types::string> m_array_string;
            std::vector<Types::type> m_array_type;
        };
        Types::Kind m_kind;
    };
}