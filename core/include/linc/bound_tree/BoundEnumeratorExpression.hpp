#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundEnumeratorExpression final : public BoundExpression
    {
    public:
        BoundEnumeratorExpression(const std::string& enumeration_name, Types::u64 enumerator_index,
            std::unique_ptr<const BoundExpression> value, const Types::type& type)
            :BoundExpression(type), m_enumerationName(std::move(enumeration_name)), m_enumeratorIndex(enumerator_index), m_value(std::move(value))
        {}

        [[nodiscard]] inline const std::string& getEnumerationName() const { return m_enumerationName; }
        [[nodiscard]] inline Types::u64 getEnumeratorIndex() const { return m_enumeratorIndex; }
        [[nodiscard]] inline const BoundExpression* const getValue() const { return m_value? m_value.get(): nullptr; }

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            return std::make_unique<const BoundEnumeratorExpression>(m_enumerationName, m_enumeratorIndex, m_value? m_value->clone(): nullptr, getType());
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Enumerator Expression";
        }
        const std::string m_enumerationName;
        const Types::u64 m_enumeratorIndex;
        const std::unique_ptr<const BoundExpression> m_value;
    };
}