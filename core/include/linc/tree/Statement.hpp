#pragma once
#include <linc/tree/Node.hpp>

namespace linc
{
    /// @brief Base AST statement node.
    class Statement : public Node
    {
    public:
        Statement(const Token terminator, const NodeInfo& info = NodeInfo{})
            :Node(info), m_terminator(terminator)
        {}

        Statement(const Token terminator, const Token::Info& token_info)
            :Node(token_info), m_terminator(terminator)
        {}

        [[nodiscard]] inline const Token& getTerminator() const { return m_terminator; }

        virtual ~Statement() = default;
        virtual std::unique_ptr<const Statement> clone() const = 0;
        const Token m_terminator;
    };
}