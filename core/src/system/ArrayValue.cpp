#include <linc/system/ArrayValue.hpp>
#include <linc/system/Value.hpp>

namespace linc
{
    const std::function<bool(ArrayValue&, std::function<void(std::size_t)>)> ArrayValue::s_stringFunction = [](ArrayValue& array, std::function<void(std::size_t)> func){
        if(array.m_type.kind == Types::type::Kind::Primitive && array.m_type.primitive == Types::type::Primitive::string)
        {
            for(std::size_t i{0ul}; i < array.m_data.size() / array.m_elementSize; ++i)
                func(i);
            return false;
        }
        return true;
    };


    ArrayValue::ArrayValue(std::size_t count, std::size_t element_size, const Types::type& type, bool initialize)
        :m_elementSize(element_size), m_type(type)
    {
        m_data.resize(m_elementSize * count);
        if(s_stringFunction(*this, [this](std::size_t i){
            new(&reinterpret_cast<Types::string*>(m_data.data())[i]) Types::string{};
        }) && initialize)
        {
            for(std::size_t i{0ul}; i < m_data.size() / m_elementSize; ++i)
                set(i, Value::fromDefault(m_type));
        }
    }

    ArrayValue::~ArrayValue()
    {
        s_stringFunction(*this, [this](std::size_t i){
            reinterpret_cast<Types::string*>(m_data.data())[i].~basic_string();
        });
    }

    auto ArrayValue::fromDefault(const Types::type& type, std::size_t count) -> ArrayValue
    {
        switch(type.kind)
        {
        case Types::type::Kind::Primitive: return fromDefault(type.primitive, count);
        case Types::type::Kind::Array: return ArrayValue(count, sizeof(ArrayValue), type);
        case Types::type::Kind::Structure: return ArrayValue(count, sizeof(std::vector<Value>), type);
        case Types::type::Kind::Enumeration: return ArrayValue(count, sizeof(EnumeratorValue), type);
        case Types::type::Kind::Function: return ArrayValue(count, sizeof(FunctionValue), type);
        default:
            throw LINC_EXCEPTION_ILLEGAL_STATE(type.kind);
        }
    }

    auto ArrayValue::fromDefault(Types::type::Primitive primitive, std::size_t count) -> ArrayValue
    {
        switch(primitive)
        {
        case Types::Kind::invalid: return ArrayValue(count, sizeof(Types::_invalid_type), Types::invalidType, false); 
        case Types::Kind::u8: return ArrayValue(count, sizeof(Types::u8), Types::fromKind(Types::Kind::u8), false);
        case Types::Kind::u16: return ArrayValue(count, sizeof(Types::u16), Types::fromKind(Types::Kind::u16), false);
        case Types::Kind::u32: return ArrayValue(count, sizeof(Types::u32), Types::fromKind(Types::Kind::u32), false);
        case Types::Kind::u64: return ArrayValue(count, sizeof(Types::u64), Types::fromKind(Types::Kind::u64), false);
        case Types::Kind::i8: return ArrayValue(count, sizeof(Types::i8), Types::fromKind(Types::Kind::i8), false);
        case Types::Kind::i16: return ArrayValue(count, sizeof(Types::i16), Types::fromKind(Types::Kind::i16), false);
        case Types::Kind::i32: return ArrayValue(count, sizeof(Types::i32), Types::fromKind(Types::Kind::i32), false);
        case Types::Kind::i64: return ArrayValue(count, sizeof(Types::i64), Types::fromKind(Types::Kind::i64), false);
        case Types::Kind::f32: return ArrayValue(count, sizeof(Types::f32), Types::fromKind(Types::Kind::f32), false);
        case Types::Kind::f64: return ArrayValue(count, sizeof(Types::f64), Types::fromKind(Types::Kind::f64), false);
        case Types::Kind::string: return ArrayValue(count, sizeof(Types::string), Types::fromKind(Types::Kind::string), false);
        case Types::Kind::type: return ArrayValue(count, sizeof(Types::type), Types::fromKind(Types::Kind::type), false);
        case Types::Kind::_char: return ArrayValue(count, sizeof(Types::_char), Types::fromKind(Types::Kind::_char), false);
        case Types::Kind::_bool: return ArrayValue(count, sizeof(Types::_bool), Types::fromKind(Types::Kind::_bool), false);
        case Types::Kind::_void: return ArrayValue(count, sizeof(Types::_void_type), Types::voidType, false);
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(primitive);
        }
    }

    void ArrayValue::set(std::size_t index, const Value& value)
    {
        switch(m_type.kind)
        {
        case Types::type::Kind::Primitive: set(index, value.getPrimitive()); break;
        case Types::type::Kind::Array: reinterpret_cast<ArrayValue*>(m_data.data())[index] = value.getArray(); break;
        case Types::type::Kind::Structure: reinterpret_cast<std::vector<Value>*>(m_data.data())[index] = value.getStructure(); break;
        case Types::type::Kind::Enumeration: reinterpret_cast<EnumeratorValue*>(m_data.data())[index] = value.getEnumerator(); break;
        case Types::type::Kind::Function: reinterpret_cast<FunctionValue*>(m_data.data())[index] = value.getFunction(); break;
        default: throw LINC_EXCEPTION_ILLEGAL_STATE(m_type.kind);
        }
    }

    void ArrayValue::set(std::size_t index, const PrimitiveValue& value)
    {
        switch(m_type.primitive)
        {
        case Types::Kind::invalid: return;
        case Types::Kind::u8: reinterpret_cast<Types::u8*>(m_data.data())[index] = value.getU8(); break;
        case Types::Kind::u16: reinterpret_cast<Types::u16*>(m_data.data())[index] = value.getU16(); break;
        case Types::Kind::u32: reinterpret_cast<Types::u32*>(m_data.data())[index] = value.getU32(); break;
        case Types::Kind::u64: reinterpret_cast<Types::u64*>(m_data.data())[index] = value.getU64(); break;
        case Types::Kind::i8: reinterpret_cast<Types::i8*>(m_data.data())[index] = value.getI8(); break;
        case Types::Kind::i16: reinterpret_cast<Types::i16*>(m_data.data())[index] = value.getI16(); break;
        case Types::Kind::i32: reinterpret_cast<Types::i32*>(m_data.data())[index] = value.getI32(); break;
        case Types::Kind::i64: reinterpret_cast<Types::i64*>(m_data.data())[index] = value.getI64(); break;
        case Types::Kind::f32: reinterpret_cast<Types::f32*>(m_data.data())[index] = value.getF32(); break;
        case Types::Kind::f64: reinterpret_cast<Types::f64*>(m_data.data())[index] = value.getF64(); break;
        case Types::Kind::string: reinterpret_cast<Types::string*>(m_data.data())[index] = value.getString(); break;
        case Types::Kind::type: reinterpret_cast<Types::type*>(m_data.data())[index] = value.getType(); break;
        case Types::Kind::_char: reinterpret_cast<Types::_char*>(m_data.data())[index] = value.getChar(); break;
        case Types::Kind::_bool: reinterpret_cast<Types::_bool*>(m_data.data())[index] = value.getBool(); break;
        case Types::Kind::_void: return;
        default: throw LINC_EXCEPTION_ILLEGAL_STATE(m_type.primitive);
        }
    }

    Value ArrayValue::get(std::size_t index) const
    {
        switch(m_type.kind)
        {
        case Types::type::Kind::Primitive: return getPrimitive(index);
        case Types::type::Kind::Array: return reinterpret_cast<const ArrayValue*>(m_data.data())[index];
        case Types::type::Kind::Structure: return Value{reinterpret_cast<const std::vector<Value>*>(m_data.data())[index]};
        case Types::type::Kind::Enumeration: return reinterpret_cast<const EnumeratorValue*>(m_data.data())[index];
        case Types::type::Kind::Function: return reinterpret_cast<const FunctionValue*>(m_data.data())[index];
        default: throw LINC_EXCEPTION_ILLEGAL_STATE(m_type.kind);
        }
    }

    PrimitiveValue ArrayValue::getPrimitive(std::size_t index) const
    {
        switch(m_type.primitive)
        {
        case Types::Kind::invalid: return PrimitiveValue::invalidValue;
        case Types::Kind::u8: return reinterpret_cast<const Types::u8*>(m_data.data())[index];
        case Types::Kind::u16: return reinterpret_cast<const Types::u16*>(m_data.data())[index];
        case Types::Kind::u32: return reinterpret_cast<const Types::u32*>(m_data.data())[index];
        case Types::Kind::u64: return reinterpret_cast<const Types::u64*>(m_data.data())[index];
        case Types::Kind::i8: return reinterpret_cast<const Types::i8*>(m_data.data())[index];
        case Types::Kind::i16: return reinterpret_cast<const Types::i16*>(m_data.data())[index];
        case Types::Kind::i32: return reinterpret_cast<const Types::i32*>(m_data.data())[index];
        case Types::Kind::i64: return reinterpret_cast<const Types::i64*>(m_data.data())[index];
        case Types::Kind::f32: return reinterpret_cast<const Types::f32*>(m_data.data())[index];
        case Types::Kind::f64: return reinterpret_cast<const Types::f64*>(m_data.data())[index];
        case Types::Kind::string: return reinterpret_cast<const Types::string*>(m_data.data())[index];
        case Types::Kind::type: return reinterpret_cast<const Types::type*>(m_data.data())[index];
        case Types::Kind::_char: return reinterpret_cast<const Types::_char*>(m_data.data())[index];
        case Types::Kind::_bool: return reinterpret_cast<const Types::_bool*>(m_data.data())[index];
        case Types::Kind::_void: return PrimitiveValue::voidValue;
        default:
            throw LINC_EXCEPTION_ILLEGAL_STATE(m_type.primitive);
        }
    }

    ArrayValue ArrayValue::operator+(const ArrayValue& other) const
    {
        if(!m_type.isCompatible(other.m_type))
            throw LINC_EXCEPTION_ILLEGAL_STATE(other);

        ArrayValue result{getCount() + other.getCount(), m_elementSize, m_type};
        if(result.m_type.kind == Types::type::Kind::Primitive && result.m_type.primitive == Types::type::Primitive::string)
        {
            for(std::size_t i{0ul}; i < getCount(); ++i)
                result.set(i, get(i));
            
            for(std::size_t i{0ul}; i < other.getCount(); ++i)
                result.set(i + getCount(), other.get(i));
        }
        else
        {
            std::memcpy(result.m_data.data(), m_data.data(), m_data.size());
            std::memcpy(result.m_data.data() + m_data.size(), other.m_data.data(), other.m_data.size());
        }
        return result;
    }

    std::string ArrayValue::toString() const
    {
        std::string result{Colors::push(Colors::Color::Purple)};
        result.push_back('[');

        for(std::size_t i{0ul}; i * m_elementSize < m_data.size(); ++i)
        {
            if(i != 0ul)
            {
                result.append(Colors::push(Colors::Color::Purple));
                result.append(", ");
                result.append(Colors::pop());
            }
            result.append(get(i).toString());
        }

        result.push_back(']');
        result.append(Colors::pop());
        return result;
    }

    std::string ArrayValue::toApplicationString() const
    {
        std::string result{'['};

        for(std::size_t i{0ul}; i * m_elementSize < m_data.size(); ++i)
        {
            if(i != 0ul)
                result.append(", ");
            result.append(get(i).toApplicationString());
        }

        result.push_back(']');
        return result;
    }
}