#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/system/Logger.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundTypeExpression final : public BoundExpression 
    {
    public:
        BoundTypeExpression(Types::Kind kind, bool is_mutable, bool is_array, const std::optional<Types::u64>& array_size);

        [[nodiscard]] inline bool getMutable() const { return m_isMutable; }
        [[nodiscard]] inline bool getArray() const { return m_isArray; }
        [[nodiscard]] inline const std::optional<Types::u64>& getArraySize() const { return m_arraySize; }
        [[nodiscard]] inline Types::Kind getKind() const { return m_kind; }
        [[nodiscard]] inline Types::type getActualType() const
        {
            return Types::type{
                .kind = m_kind,
                .isMutable = m_isMutable,
                .isArray = m_isArray,
                .arraySize = m_arraySize
            };
        }

        virtual std::unique_ptr<const BoundExpression> cloneConst() const final override;
    private:
        virtual std::string toStringInner() const final override;

        const Types::Kind m_kind;
        const bool m_isMutable, m_isArray;
        const std::optional<Types::u64> m_arraySize;
    };
}