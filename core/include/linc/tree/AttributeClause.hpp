#pragma once
#include <linc/tree/Clause.hpp>
#include <linc/tree/NodeListClause.hpp>

namespace linc
{
    class LiteralExpression;
    class AttributeArgumentClause final : public Clause<AttributeArgumentClause>
    {
    public:
        AttributeArgumentClause(const Token& left_parenthesis, const Token& right_parenthesis,
            std::unique_ptr<const NodeListClause<LiteralExpression>> arguments);
        ~AttributeArgumentClause();

        [[nodiscard]] inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        [[nodiscard]] inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        [[nodiscard]] inline const NodeListClause<LiteralExpression>* getArguments() const { return m_arguments.get(); }

        virtual std::unique_ptr<const AttributeArgumentClause> clone() const final override;
    private:
        const Token m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const NodeListClause<LiteralExpression>> m_arguments;
    };

    class AttributeClause final : public Clause<AttributeClause>
    {
    public:
        AttributeClause(const Token& attribute_specifier, const Token& identifier, std::unique_ptr<const AttributeArgumentClause> argument_clause);
        ~AttributeClause();

        [[nodiscard]] inline const Token& getAttributeSpecifier() const { return m_attributeSpecifier; }
        [[nodiscard]] inline const Token& getIdentifier() const { return m_identifier; }
        [[nodiscard]] inline const AttributeArgumentClause* const getArguments() const { return m_argumentClause? m_argumentClause.get(): nullptr; }

        virtual std::unique_ptr<const AttributeClause> clone() const final override;
    private:
        const Token m_attributeSpecifier, m_identifier;
        const std::unique_ptr<const AttributeArgumentClause> m_argumentClause;
    };
}