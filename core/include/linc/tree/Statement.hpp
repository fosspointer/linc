#pragma once
#include <linc/tree/Node.hpp>

namespace linc
{
    /// @brief Base AST statement node.
    class Statement : public Node
    {
    public:
        Statement(const NodeInfo& info = NodeInfo{})
            :Node(info)
        {}

        Statement(const Token::Info& token_info)
            :Node(token_info)
        {}

        virtual ~Statement() = default;
        virtual std::unique_ptr<const Statement> clone() const = 0;
    };
}