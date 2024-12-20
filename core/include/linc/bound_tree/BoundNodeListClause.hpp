#pragma once
#include <linc/bound_tree/BoundClause.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    template <typename T>
    class BoundNodeListClause final : public BoundClause<BoundNodeListClause<T>>
    {
    public:
        BoundNodeListClause(std::vector<std::unique_ptr<const T>> nodes, const Token::Info& info)
            :BoundClause<BoundNodeListClause>(info), m_nodes(std::move(nodes))
        {}

        inline const std::vector<std::unique_ptr<const T>>& getList() const { return m_nodes; }
        std::unique_ptr<const BoundNodeListClause<T>> clone() const final override
        {
            std::vector<std::unique_ptr<const T>> nodes;
            nodes.reserve(m_nodes.size());
            for(const auto& node: m_nodes)
            {
                auto node_cast = Types::uniqueCast<const T>(node->clone());
                nodes.push_back(std::move(node_cast));
            }
            return std::make_unique<const BoundNodeListClause<T>>(std::move(nodes), this->getInfo());
        }
    private:
        const std::vector<std::unique_ptr<const T>> m_nodes;
    };
}