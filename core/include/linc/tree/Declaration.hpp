#pragma once
#include <linc/tree/Node.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/AttributeClause.hpp>

namespace linc
{
    /// @brief Base AST declaration node.
    class Declaration : public Node
    {
    public:
        using AttributeMap = std::unordered_map<std::string, std::unique_ptr<const AttributeClause>>;
        Declaration(std::unique_ptr<const IdentifierExpression> identifier, AttributeMap attributes, const NodeInfo& info = NodeInfo{})
            :Node(info), m_identifier(std::move(identifier)), m_attributes(std::move(attributes))
        {}

        Declaration(std::unique_ptr<const IdentifierExpression> identifier, AttributeMap attributes, const Token::Info& token_info)
            :Node(token_info), m_identifier(std::move(identifier)), m_attributes(std::move(attributes))
        {}

        [[nodiscard]] inline const AttributeMap& getAttributes() const { return m_attributes; }
        [[nodiscard]] inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }

        virtual ~Declaration() = default;
        virtual std::unique_ptr<const Declaration> clone() const = 0;
        virtual std::unique_ptr<const Declaration> cloneRename(std::unique_ptr<const IdentifierExpression> identifier) const
        {
            auto declaration = clone();
            declaration->m_identifier = std::move(identifier);
            return declaration;
        }
    protected:
        mutable std::unique_ptr<const IdentifierExpression> m_identifier;
        const AttributeMap m_attributes;
    };
}