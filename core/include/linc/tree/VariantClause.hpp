#pragma once
#include <linc/Include.hpp>
#include <linc/tree/Clause.hpp>

namespace linc
{
    template <typename FIRST, typename SECOND>
    class VariantClause final : public Clause<VariantClause<FIRST, SECOND>>
    {
    public:
        VariantClause(std::unique_ptr<const FIRST> first)
            :Clause<VariantClause<FIRST, SECOND>>(first->getTokenInfo()), m_variant(std::move(first))
        {
            this->addTokens(std::get<0ul>(m_variant)->getTokens());
        }

        VariantClause(std::unique_ptr<const SECOND> second)
            :Clause<VariantClause<FIRST, SECOND>>(second->getTokenInfo()), m_variant(std::move(second))
        {
            this->addTokens(std::get<1ul>(m_variant)->getTokens());
        }

        const FIRST* const getIfFirst() const
        {
            if(const auto* value = std::get_if<0ul>(&m_variant)) return value->get();
            else return nullptr;
        }

        const SECOND* const getIfSecond() const
        {
            if(const auto* value = std::get_if<1ul>(&m_variant)) return value->get();
            else return nullptr;
        }

        const FIRST* const getFirst() const { return std::get<0ul>(m_variant).get(); }
        const SECOND* const getSecond() const { return std::get<1ul>(m_variant).get(); }

        std::unique_ptr<const VariantClause> clone() const final override
        {
            if(const auto* value = std::get_if<0ul>(&m_variant))
            {
                auto node_cast = Types::uniqueCast<const FIRST>(value->get()->clone());
                return std::make_unique<const VariantClause>(std::move(node_cast));
            }
            else
            {
                auto node_cast = Types::uniqueCast<const SECOND>(std::get<1ul>(m_variant)->clone());
                return std::make_unique<const VariantClause>(std::move(node_cast));
            }
        }
    private:
        const std::variant<std::unique_ptr<const FIRST>, std::unique_ptr<const SECOND>> m_variant;
    };
}