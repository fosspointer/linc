#include <linc/system/EnumeratorValue.hpp>
#include <linc/system/Value.hpp>

namespace linc
{
    EnumeratorValue::EnumeratorValue(const std::string& name, Types::u64 index, Value value)
        :m_name(name), m_index(index), m_value(std::make_unique<Value>(std::move(value)))
    {}

    EnumeratorValue::EnumeratorValue(const EnumeratorValue& other)
        :m_name(other.m_name), m_index(other.m_index), m_value(std::make_unique<Value>(*other.m_value))
    {}

    EnumeratorValue::EnumeratorValue(EnumeratorValue&& other)
        :m_name(std::move(other.m_name)), m_index(other.m_index), m_value(std::move(other.m_value))
    {}

    bool EnumeratorValue::operator==(const EnumeratorValue& other) const
    {
        return m_name == other.m_name && m_index == other.m_index && *m_value == *other.m_value;
    }
}