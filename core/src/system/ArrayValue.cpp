#include <linc/system/ArrayValue.hpp>
#include <linc/system/Value.hpp>

namespace linc
{
    Value ArrayValue::get(std::size_t index) const
    {
        if(m_isNested)
            return m_array_array.at(index);
        
        switch(m_kind)
        {
        case Types::Kind::_void: return PrimitiveValue::voidValue; break;
        case Types::Kind::_bool: return PrimitiveValue(m_array__bool.at(index)); break;
        case Types::Kind::_char: return PrimitiveValue(m_array__char.at(index)); break;
        case Types::Kind::u8: return PrimitiveValue(m_array_u8.at(index)); break;
        case Types::Kind::u16: return PrimitiveValue(m_array_u16.at(index)); break;
        case Types::Kind::u32: return PrimitiveValue(m_array_u32.at(index)); break;
        case Types::Kind::u64: return PrimitiveValue(m_array_u64.at(index)); break;
        case Types::Kind::i8: return PrimitiveValue(m_array_i8.at(index)); break;
        case Types::Kind::i16: return PrimitiveValue(m_array_i16.at(index)); break;
        case Types::Kind::i32: return PrimitiveValue(m_array_i32.at(index)); break;
        case Types::Kind::i64: return PrimitiveValue(m_array_i64.at(index)); break;
        case Types::Kind::f32: return PrimitiveValue(m_array_f32.at(index)); break;
        case Types::Kind::f64: return PrimitiveValue(m_array_f64.at(index)); break;
        case Types::Kind::string: return PrimitiveValue(m_array_string.at(index)); break;
        case Types::Kind::type: return PrimitiveValue(m_array_type.at(index)); break;
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_kind);
        }
    }

    void ArrayValue::set(std::size_t index, const Value& value)
    {
        if(m_isNested)
        {
            m_array_array.at(index) = value.getArray();
            return;
        }

        switch(m_kind)
        {
        case Types::Kind::_void: m_array__void.at(index) = value.getPrimitive().getVoid(); break;
        case Types::Kind::_bool: m_array__bool.at(index) = value.getPrimitive().getBool(); break;
        case Types::Kind::_char: m_array__char.at(index) = value.getPrimitive().getChar(); break;
        case Types::Kind::u8: m_array_u8.at(index) = value.getPrimitive().getU8(); break;
        case Types::Kind::u16: m_array_u16.at(index) = value.getPrimitive().getU16(); break;
        case Types::Kind::u32: m_array_u32.at(index) = value.getPrimitive().getU32(); break;
        case Types::Kind::u64: m_array_u64.at(index) = value.getPrimitive().getU64(); break;
        case Types::Kind::i8: m_array_i8.at(index) = value.getPrimitive().getI8(); break;
        case Types::Kind::i16: m_array_i16.at(index) = value.getPrimitive().getI16(); break;
        case Types::Kind::i32: m_array_i32.at(index) = value.getPrimitive().getI32(); break;
        case Types::Kind::i64: m_array_i64.at(index) = value.getPrimitive().getI64(); break;
        case Types::Kind::f32: m_array_f32.at(index) = value.getPrimitive().getF32(); break;
        case Types::Kind::f64: m_array_f64.at(index) = value.getPrimitive().getF64(); break;
        case Types::Kind::string: m_array_string.at(index) = value.getPrimitive().getString(); break;
        case Types::Kind::type: m_array_type.at(index) = value.getPrimitive().getType(); break;
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_kind);
        }
    }

    void ArrayValue::push(const Value& value)
    {
        if(m_isNested)
        {
            m_array_array.push_back(value.getArray());
            return;
        }

        switch(m_kind)
        {
        case Types::Kind::_void: pushVoid(value.getPrimitive().getIfVoid().value()); break;
        case Types::Kind::_bool: pushBool(value.getPrimitive().getIfBool().value()); break;
        case Types::Kind::_char: pushChar(value.getPrimitive().getIfChar().value()); break;
        case Types::Kind::u8: pushU8(value.getPrimitive().getIfU8().value()); break;
        case Types::Kind::u16: pushU16(value.getPrimitive().getIfU16().value()); break;
        case Types::Kind::u32: pushU32(value.getPrimitive().getIfU32().value()); break;
        case Types::Kind::u64: pushU64(value.getPrimitive().getIfU64().value()); break;
        case Types::Kind::i8: pushI8(value.getPrimitive().getIfI8().value()); break;
        case Types::Kind::i16: pushI16(value.getPrimitive().getIfI16().value()); break;
        case Types::Kind::i32: pushI32(value.getPrimitive().getIfI32().value()); break;
        case Types::Kind::i64: pushI64(value.getPrimitive().getIfI64().value()); break;
        case Types::Kind::f32: pushF32(value.getPrimitive().getIfF32().value()); break;
        case Types::Kind::f64: pushF64(value.getPrimitive().getIfF64().value()); break;
        case Types::Kind::string: pushString(value.getPrimitive().getIfString().value()); break;
        case Types::Kind::type: pushType(value.getPrimitive().getIfType().value()); break;
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_kind);
        }
    }
}