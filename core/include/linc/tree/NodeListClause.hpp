#pragma once
#include <linc/tree/Clause.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    template <typename T>
    class NodeListClause final : public Clause<NodeListClause<T>>
    {
    public:
        struct DelimitedNode
        {
            std::optional<const Token> delimiter;
            std::unique_ptr<const T> node;
        };

        NodeListClause(std::vector<DelimitedNode> nodes, const Token::Info& info)
            :Clause<NodeListClause>(info), m_nodes(std::move(nodes))
        {
            for(const auto& node: m_nodes)
            {
                this->addTokens(node.node->getTokens());
                if(node.delimiter)
                    this->addToken(*node.delimiter);
            }
        }

        inline const std::vector<DelimitedNode>& getList() const { return m_nodes; }

        std::unique_ptr<const NodeListClause<T>> clone() const final override
        {
            std::vector<DelimitedNode> nodes;
            nodes.reserve(m_nodes.size());
            for(const auto& node: m_nodes)
            {
                auto node_cast = Types::uniqueCast<const T>(node.node->clone());
                nodes.push_back(DelimitedNode{
                    .delimiter = node.delimiter,
                    .node = std::move(node_cast)
                });
            }
            return std::make_unique<const NodeListClause<T>>(std::move(nodes), this->getTokenInfo());
        }
    private:
        const std::vector<DelimitedNode> m_nodes;
    };
}