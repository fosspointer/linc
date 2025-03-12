#pragma once
#include <linc/tree/Clause.hpp>

namespace linc
{
    class NamespaceClause final: Clause<NamespaceClause>
    {
    public:
        struct Item final
        {
            Token namespaceIdentifier, namespaceAccess;
        };

        NamespaceClause(std::vector<Item> items)
            :Clause<NamespaceClause>(items.empty()? Token::Info{}: items[0ul].namespaceIdentifier.info), m_items(std::move(items))
        {
            for(const auto& item: m_items)
            {
                addToken(item.namespaceIdentifier);
                addToken(item.namespaceAccess);
            }
        }

        [[nodiscard]] inline const std::vector<Item>& getItems() const { return m_items; }
        [[nodiscard]] std::string asString() const
        {
            std::string actual_namespace;
            for(const auto& item: m_items)
                actual_namespace += item.namespaceIdentifier.value.value_or(std::string{}) + "::";
            return actual_namespace;
        }
        virtual std::unique_ptr<const NamespaceClause> clone() const final override
        {
            return std::make_unique<const NamespaceClause>(m_items);
        }
    private:
        const std::vector<Item> m_items;
    };
}