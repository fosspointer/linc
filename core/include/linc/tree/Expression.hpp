#pragma once
#include <linc/tree/Node.hpp>

namespace linc
{
    class Expression : public Node
    {
    public:
        Expression(const NodeInfo& info = NodeInfo{})
            :Node(info)
        {}

        Expression(const Token::Info& token_info)
            :Node(token_info)
        {}

        virtual ~Expression() = default;
        virtual std::unique_ptr<const Expression> clone() const = 0;
    };
}