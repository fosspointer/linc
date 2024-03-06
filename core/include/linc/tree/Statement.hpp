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
        virtual std::unique_ptr<const Statement> cloneConst() const = 0;
    };
}