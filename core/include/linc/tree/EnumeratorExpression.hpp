#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class EnumeratorExpression final : public Expression 
    {
    public:
        EnumeratorExpression(const Token& namespace_access_token, const std::optional<Token>& left_parenthesis, const std::optional<Token>& right_parenthesis,
            std::unique_ptr<const IdentifierExpression> enumeration_identifier, std::unique_ptr<const IdentifierExpression> identifier,
            std::unique_ptr<const Expression> value)
            :Expression(namespace_access_token.info), m_namespaceAccessToken(namespace_access_token), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
            m_enumerationIdentifier(std::move(enumeration_identifier)), m_identifier(std::move(identifier)), m_value(std::move(value))
        {
            addTokens(m_enumerationIdentifier->getTokens());
            addToken(m_namespaceAccessToken);
            addTokens(m_identifier->getTokens());
            if(m_leftParenthesis && m_rightParenthesis && value)
            {
                addToken(*m_leftParenthesis);
                addTokens(value->getTokens());
                addToken(*m_rightParenthesis);
            }
        }

        inline const Token& getNamespaceAccessToken() const { return m_namespaceAccessToken; }
        inline const std::optional<Token>& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const std::optional<Token>& getRightParenthesis() const { return m_rightParenthesis; }
        inline const IdentifierExpression* const getEnumerationIdentifier() const { return m_enumerationIdentifier.get(); }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const Expression* const getValue() const { return m_value? m_value.get(): nullptr; }
        
        virtual std::unique_ptr<const Expression> clone() const final override
        {
            auto enumeration_identifier = Types::uniqueCast<const IdentifierExpression>(m_enumerationIdentifier->clone());
            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const EnumeratorExpression>(m_namespaceAccessToken, m_leftParenthesis, m_rightParenthesis,
                std::move(enumeration_identifier), std::move(identifier), m_value? m_value->clone(): nullptr);
        }
    private:
        const Token m_namespaceAccessToken;
        const std::optional<Token> m_leftParenthesis, m_rightParenthesis;
        const std::unique_ptr<const IdentifierExpression> m_enumerationIdentifier, m_identifier;
        const std::unique_ptr<const Expression> m_value;
    };
}