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

        Declaration(const Token::Info& token_info)
            :Node(token_info)
        {}

        virtual ~Declaration() = default;
        virtual std::unique_ptr<const Declaration> clone() const = 0;
    };
}