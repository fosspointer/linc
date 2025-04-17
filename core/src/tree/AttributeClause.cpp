#include <linc/tree/AttributeClause.hpp>
#include <linc/tree/LiteralExpression.hpp>

namespace linc
{
    AttributeArgumentClause::AttributeArgumentClause(const Token& left_parenthesis, const Token& right_parenthesis,
        std::unique_ptr<const NodeListClause<LiteralExpression>> arguments)
        :Clause<AttributeArgumentClause>(left_parenthesis.info), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
        m_arguments(std::move(arguments))
    {
        addToken(m_leftParenthesis);
        addTokens(m_arguments->getTokens());
        addToken(m_rightParenthesis);
    }
        
    AttributeArgumentClause::~AttributeArgumentClause() = default;

    std::unique_ptr<const AttributeArgumentClause> AttributeArgumentClause::clone() const
    {
        return std::make_unique<const AttributeArgumentClause>(m_leftParenthesis, m_rightParenthesis, m_arguments->clone());
    }

    AttributeClause::AttributeClause(const Token& attribute_specifier, const Token& identifier, std::unique_ptr<const AttributeArgumentClause> argument_clause)
        :Clause<AttributeClause>(attribute_specifier.info), m_attributeSpecifier(attribute_specifier), m_identifier(identifier),
        m_argumentClause(std::move(argument_clause))
    {
        addToken(m_attributeSpecifier);
        addToken(m_identifier);
        if(m_argumentClause)
            addTokens(m_argumentClause->getTokens());
    }
    AttributeClause::~AttributeClause() = default;

    std::unique_ptr<const AttributeClause> AttributeClause::clone() const
    {
        return std::make_unique<const AttributeClause>(m_attributeSpecifier, m_identifier, m_argumentClause? m_argumentClause->clone(): nullptr);
    }
}