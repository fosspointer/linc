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
        case Kind::Structure: return m_structure op other.m_structure; \
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
            Invalid, Primitive, Array, Structure
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

        explicit Value(const std::vector<Value>& value)
            :m_kind(Kind::Structure)
        {
            new (&m_structure) std::vector<Value>{copyStructure(value)};
        }

        Value(const Value& other)
            :m_kind(other.m_kind)
        {
            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{other.m_primitive}; break;
            case Kind::Array: new (&m_array) ArrayValue{other.m_array}; break;
            case Kind::Structure: new (&m_structure) std::vector<Value>{copyStructure(other.m_structure)}; break;
            default:
                throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }

        Value& operator=(const Value& other)
        {
            m_kind = other.m_kind;

            switch(m_kind)
            {
            case Kind::Primitive: new (&m_primitive) PrimitiveValue{other.m_primitive}; break;
            case Kind::Array: new (&m_array) ArrayValue{other.m_array}; break;
            case Kind::Structure: new (&m_structure) std::vector<Value>{copyStructure(other.m_structure)}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
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
            case Kind::Structure: new (&m_structure) std::vector<Value>{copyStructure(other.m_structure)}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }

        ~Value()
        {
            switch(m_kind)
            {
            case Kind::Primitive: m_primitive.~PrimitiveValue(); break;
            case Kind::Array: m_array.~ArrayValue(); break;
            case Kind::Structure: m_structure.~vector(); break;
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
            case Kind::Structure: new (&m_structure) std::vector<Value>{copyStructure(other.m_structure)}; break;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }

            return *this;
        }

        static Value fromDefault(const Types::type& type)
        {
            switch(type.kind)
            {
            case Types::type::Kind::Primitive: return PrimitiveValue::fromDefault(type.primitive);
            case Types::type::Kind::Array: return ArrayValue::fromDefault(*type.array.base_type, type.array.count.value_or(0ul));
            case Types::type::Kind::Structure:
            {
                std::vector<Value> values;
                values.reserve(type.structure.size());
                
                for(const auto& member: type.structure)
                    values.push_back(fromDefault(*member.second));
                
                return Value(values);
            }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
            }
        }

        Value operator+(const Value& other) const
        {
            if(other.m_kind != m_kind)
                throw LINC_EXCEPTION_INVALID_INPUT("Invalid operator for array and primitive operands.");
            switch(m_kind)
            {
            case Kind::Primitive: return m_primitive + other.m_primitive;
            case Kind::Array: return m_array + other.m_array;
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }

        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(++, Value, )
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(--, Value, )
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(-, Value, const)
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(+, Value, const)
        LINC_VALUE_OPERATOR_UNARY_PRIMITIVE(~, Value, const)

        LINC_VALUE_OPERATOR_GENERIC(==, bool, const)
        LINC_VALUE_OPERATOR_GENERIC(!=, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(-, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(*, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(/, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(%, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(>, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(<, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(>=, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(<=, bool, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(<<, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(>>, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(&, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(|, Value, const)
        LINC_VALUE_OPERATOR_PRIMITIVE(^, Value, const)

        inline const PrimitiveValue& getPrimitive() const { return m_primitive; }
        inline const ArrayValue& getArray() const { return m_array; }
        inline const std::vector<Value>& getStructure() const { return m_structure; }

        inline PrimitiveValue& getPrimitive() { return m_primitive; }
        inline ArrayValue& getArray() { return m_array; }
        inline std::vector<Value>& getStructure() { return m_structure; }

        inline std::optional<PrimitiveValue> getIfPrimitive() const
        {
            return m_kind == Kind::Primitive? std::make_optional(m_primitive): std::nullopt;
        }
        
        inline std::optional<ArrayValue> getIfArray() const
        {
            return m_kind == Kind::Array? std::make_optional(m_array): std::nullopt;
        }

        inline std::optional<std::vector<Value>> getIfStructure() const
        {
            return m_kind == Kind::Structure? std::make_optional(m_structure): std::nullopt;
        }

        [[nodiscard]] inline std::string toApplicationString() const 
        {
            switch(m_kind)
            {
            case Kind::Primitive: return m_primitive.toApplicationString();
            case Kind::Array: return m_array.toApplicationString();
            case Kind::Structure:
            {
                std::string result;
                result.push_back('{');
                for(Types::type::Structure::size_type i{0ul}; i < m_structure.size(); ++i)
                    result.append((i == 0ul? "": ", ") + m_structure[i].toApplicationString());
                    
                result.push_back('}');
                return result;
            }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }

        [[nodiscard]] inline std::string toString() const 
        {
            switch(m_kind)
            {
            case Kind::Primitive: return m_primitive.toString();
            case Kind::Array: return m_array.toString();
            case Kind::Structure:
            {
                std::string result;
                result.push_back('{');
                for(Types::type::Structure::size_type i{0ul}; i < m_structure.size(); ++i)
                    result.append((i == 0ul? "": ", ") + m_structure[i].toString());
                    
                result.push_back('}');
                return result;
            }
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Value::Kind);
            }
        }
    private:
        static std::vector<Value> copyStructure(const std::vector<Value>& structure) 
        {
            std::vector<Value> values;
            values.reserve(structure.size());
            
            for(const auto& member: structure)
                values.push_back(member);
            
            return values;
        }

        union
        {
            PrimitiveValue m_primitive;
            ArrayValue m_array;
            std::vector<Value> m_structure;
        };
        Kind m_kind;
    };
}
