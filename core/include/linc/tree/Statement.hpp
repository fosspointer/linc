#pragma once
#include <linc/tree/Node.hpp>

namespace linc
{
    class Statement : public Node
    {
    public:
        Statement(const NodeInfo& info = NodeInfo{})
            :Node(info)
        {}

        virtual ~Statement() = default;
    };
}