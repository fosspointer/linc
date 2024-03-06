#pragma once
#include <linc/system/PrimitiveValue.hpp>
#include <linc/system/ArrayValue.hpp>

#define LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(op, return_type, const_keyword) \
return_type operator op() const_keyword \
    { \
        switch(m_kind) \
        { \
        case Kind::Primitive: return op m_primitive; \
        default: throw LINC_EXCEPTION_INVALID_INPUT("Invalid operand to primitive-only operator."); \
        } \
    }

#define LINC_VALUE_OPERATOR_UNARY_GENERIC(op, return_type, const_keyword) \
return_type operator op() const_keyword \
    { \
        switch(m_kind) \
        { \
        case Kind::Primitive: return op m_primitive; \
        case Kind::Array: return op m_array; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind); \
        } \
    }

#define LINC_VALUE_OPERATOR_PRIMITIVE(op, return_type, const_keyword) \
    return_type operator op(const Value& other) const_keyword \
    { \
        if(other.m_kind != m_kind) \
            throw LINC_EXCEPTION_INVALID_INPUT("Invalid operator for array and primitive operands."); \
        switch(m_kind) \
        { \
        case Kind::Primitive: return m_primitive op other.m_primitive; \
        default: throw LINC_EXCEPTION_INVALID_INPUT("Invalid operand(s) to primitive-only operator."); \
        } \
    }

#define LINC_VALUE_OPERATOR_GENERIC(op, return_type, const_keyword) \
    return_type operator op(const Value& other) const_keyword \
    { \
        if(other.m_kind != m_kind) \
            throw LINC_EXCEPTION_INVALID_INPUT("Invalid operator for array and primitive operands."); \
        switch(m_kind) \
        { \
        case Kind::Primitive: return m_primitive op other.m_primitive; \
        case Kind::Array: return m_array op other.m_array; \
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind); \
        } \
    }

namespace linc
{
    class Value final
    {
    public:
        enum class Kind: char
        {
            Invalid, Primitive, Array
        };

        Value(const PrimitiveValue& value)
            :m_kind(Kind::Primitive)
        {
            new (&m_primitive) PrimitiveValue{value};
        }

        Value(const ArrayValue& value)
            :m_kind(Kind::Array)
        {
            new (&m_array) ArrayValue{value};
        }

        Value(const Value& other)
            :m_kind(other.m_kind)
        {
            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{other.m_primitive}; break;
            case Kind::Array: new (&m_array) ArrayValue{other.m_array}; break;
            }
        }

        Value& operator=(const Value& other)
        {
            m_kind = other.m_kind;

            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{other.m_primitive}; break;
            case Kind::Array: new (&m_array) ArrayValue{other.m_array}; break;
            }

            return *this;
        }

        Value(Value&& other)
            :m_kind(other.m_kind)
        {
            other.m_kind = Kind::Invalid;

            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{std::move(other.m_primitive)}; break;
            case Kind::Array: new (&m_array) ArrayValue{std::move(other.m_array)}; break;
            }
        }

        ~Value()
        {
            switch(m_kind)
            {
            case Kind::Primitive: m_primitive.~PrimitiveValue(); break;
            case Kind::Array: m_array.~ArrayValue(); break;
            }
        }

        Value& operator=(Value&& other)
        {
            m_kind = other.m_kind;
            other.m_kind = Kind::Invalid;

            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{std::move(other.m_primitive)}; break;
            case Kind::Array: new (&m_array) ArrayValue{std::move(other.m_array)}; break;
            }

            return *this;
        }

        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(++, Value, )
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(--, Value, )
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(-, Value, const)
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(+, Value, const)

        LINC_VALUE_OPERATOR_GENERIC(==, bool, const)
        LINC_VALUE_OPERATOR_GENERIC(!=, bool, const)
        LINC_VALUE_OPERATOR_GENERIC(+, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(-, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(*, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(/, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(>, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(<, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(>=, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(<=, bool, const)

        inline const PrimitiveValue& getPrimitive() const { return m_primitive; }
        inline const ArrayValue& getArray() const { return m_array; }

        inline PrimitiveValue& getPrimitive() { return m_primitive; }
        inline ArrayValue& getArray() { return m_array; }

        inline std::optional<PrimitiveValue> getIfPrimitive() const
        {
            return m_kind == Kind::Primitive? std::make_optional(m_primitive): std::nullopt;
        }
        
        inline std::optional<ArrayValue> getIfArray() const
        {
            return m_kind == Kind::Array? std::make_optional(m_array): std::nullopt;
        }

        std::string toString() const 
        {
            switch(m_kind)
            {
            case Kind::Primitive: return m_primitive.toString();
            case Kind::Array: return m_array.toString();
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }
    private:
        union
        {
            PrimitiveValue m_primitive;
            ArrayValue m_array;
        };
        Kind m_kind;
    };
}