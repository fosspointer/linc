#pragma once
#include <linc/tree/Node.hpp>

namespace linc
{
    class Declaration : public Node
    {
    public:
        Declaration(NodeInfo info = NodeInfo{})
            :Node(info)
        {}

        virtual ~Declaration() = default;
        virtual std::unique_ptr<const Declaration> clone_const() const = 0;
    };
}