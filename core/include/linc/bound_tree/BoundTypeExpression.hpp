#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/system/Logger.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundTypeExpression final : public BoundExpression 
    {
    public:
        using BoundArraySpecifiers = std::vector<std::optional<Types::u64>>;
        using Base = std::variant<Types::type::Primitive, Types::type::Structure>;
        BoundTypeExpression(Types::type::Primitive primitive, bool is_mutable, BoundArraySpecifiers specifiers);
        BoundTypeExpression(Types::type::Structure structure, bool is_mutable, BoundArraySpecifiers specifiers);

        [[nodiscard]] inline bool getMutable() const { return m_isMutable; }
        [[nodiscard]] inline bool getArray() const { return !m_arraySpecifiers.empty(); }
        [[nodiscard]] inline const BoundArraySpecifiers& getArraySpecifiers() const { return m_arraySpecifiers; }
        [[nodiscard]] inline const Base& getBase() const { return m_base; }
        [[nodiscard]] inline Types::type getActualType() const
        {
            std::unique_ptr<Types::type> base{nullptr};
            if(auto kind = std::get_if<Types::type::Primitive>(&m_base))
                base = std::make_unique<Types::type>(*kind, m_isMutable);
            else
                base = std::make_unique<Types::type>(Types::type::cloneStructure(&std::get<Types::type::Structure>(m_base), m_isMutable), m_isMutable);

            if(m_arraySpecifiers.empty())
                return *base;
            else
            {
                Types::type result(Types::type::Array{.base_type = base->clone(),
                    .count = m_arraySpecifiers.back()}, m_isMutable);
                auto cursor = &result;

                for(std::size_t i{m_arraySpecifiers.size()}; i != 1ul; --i)
                {
                    cursor->array.base_type = std::make_unique<Types::type>(Types::type::Array{
                        .base_type = i == 2ul? base->clone() :nullptr,
                        .count = m_arraySpecifiers[i - 2ul]
                    }, m_isMutable);
                    cursor = const_cast<Types::type*>(cursor->array.base_type.get());
                }

                return result;
            }
        }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const Base m_base;
        const bool m_isMutable;
        const BoundArraySpecifiers m_arraySpecifiers;
    };
}