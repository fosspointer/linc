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

        virtual ~Expression() = default;
    };
}