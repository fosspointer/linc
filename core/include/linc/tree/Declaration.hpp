#pragma once
#include <linc/tree/Node.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    /// @brief Base AST declaration node.
    class Declaration : public Node
    {
    public:
        Declaration(std::unique_ptr<const IdentifierExpression> identifier, NodeInfo info = NodeInfo{})
            :Node(info), m_identifier(std::move(identifier))
        {}

        Declaration(std::unique_ptr<const IdentifierExpression> identifier, const Token::Info& token_info)
            :Node(token_info), m_identifier(std::move(identifier))
        {}

        [[nodiscard]] inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }

        virtual ~Declaration() = default;
        virtual std::unique_ptr<const Declaration> clone() const = 0;
        std::unique_ptr<const Declaration> cloneRename(std::unique_ptr<const IdentifierExpression> identifier) const
        {
            auto declaration = clone();
            declaration->m_identifier = std::move(identifier);
            return declaration;
        }
    protected:
        mutable std::unique_ptr<const IdentifierExpression> m_identifier;
    };
}