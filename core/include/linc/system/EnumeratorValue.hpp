#pragma once
#include <linc/system/Types.hpp>

namespace linc
{
    class EnumeratorValue final
    {
    public:
        EnumeratorValue(const std::string& name, Types::u64 index, class Value value);
        EnumeratorValue(const EnumeratorValue& other);
        EnumeratorValue(EnumeratorValue&& other);
        EnumeratorValue& operator=(const EnumeratorValue& other);
        EnumeratorValue& operator=(EnumeratorValue&& other);
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const Types::u64& getIndex() const { return m_index; }
        [[nodiscard]] inline const class Value& getValue() const { return *m_value; }
        bool operator==(const EnumeratorValue& other) const;
        bool operator!=(const EnumeratorValue& other) const = default;
    private:
        std::string m_name;
        Types::u64 m_index;
        std::unique_ptr<class Value> m_value;
    };
}